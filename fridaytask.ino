#include <WiFi.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#define MAX_DEVICES 20
#define DEAUTH_THRESHOLD 20
#define QUEUE_SIZE 128

// Device structure
struct Device {
  uint8_t mac[6];
  uint32_t last_seen;
  uint32_t last_reset;
  uint16_t deauth_count;
  int8_t rssi;
  bool is_attacker;
};

Device devices[MAX_DEVICES];
int device_count = 0;

// Queue structures
uint8_t queue_mac[QUEUE_SIZE][6];
int8_t queue_rssi[QUEUE_SIZE];
uint8_t queue_subtype[QUEUE_SIZE];
volatile int queue_head = 0;
volatile int queue_tail = 0;
volatile int queue_count = 0;

volatile uint32_t total_packets = 0;
volatile uint32_t deauth_alerts = 0;

// Helper function
void mac_to_str(uint8_t *mac, char *out) {
  sprintf(out, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// Find device by MAC
int find_device(uint8_t *mac) {
  for (int i = 0; i < device_count; i++) {
    if (memcmp(devices[i].mac, mac, 6) == 0) return i;
  }
  return -1;
}

// Promiscuous callback (ISR)
void IRAM_ATTR promiscuous_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  if (pkt->rx_ctrl.sig_len < 24) return;
  
  uint8_t *payload = pkt->payload;
  uint16_t frame_ctrl = *(uint16_t *)payload;
  uint8_t frame_type = (frame_ctrl >> 2) & 0x3;
  uint8_t subtype = (frame_ctrl >> 4) & 0xF;
  
  // Skip data frames
  if (frame_type == 2) return;
  
  // Source MAC is at offset 10 (addr2)
  uint8_t *src_mac = payload + 10;
  
  // Add to queue if space available
  int next_head = (queue_head + 1) % QUEUE_SIZE;
  if (next_head != queue_tail && queue_count < QUEUE_SIZE) {
    memcpy(queue_mac[queue_head], src_mac, 6);
    queue_rssi[queue_head] = pkt->rx_ctrl.rssi;
    queue_subtype[queue_head] = subtype;
    queue_head = next_head;
    queue_count++;
    total_packets++;
  }
}

// Process a single packet from queue
void process_packet() {
  if (queue_count == 0) return;
  
  // Dequeue
  uint8_t mac[6];
  memcpy(mac, queue_mac[queue_tail], 6);
  int8_t rssi = queue_rssi[queue_tail];
  uint8_t subtype = queue_subtype[queue_tail];
  
  queue_tail = (queue_tail + 1) % QUEUE_SIZE;
  queue_count--;
  
  uint32_t now = millis();
  int idx = find_device(mac);
  
  // New device?
  if (idx == -1 && device_count < MAX_DEVICES) {
    idx = device_count;
    memcpy(devices[idx].mac, mac, 6);
    devices[idx].last_seen = now;
    devices[idx].last_reset = now;
    devices[idx].deauth_count = 0;
    devices[idx].is_attacker = false;
    devices[idx].rssi = rssi;
    device_count++;
    
    char mac_str[18];
    mac_to_str(mac, mac_str);
    Serial.printf("[NEW] %s | RSSI: %d dBm\n", mac_str, rssi);
  }
  
  if (idx == -1 || idx >= device_count) return;
  
  // Update existing device
  devices[idx].last_seen = now;
  devices[idx].rssi = rssi;
  
  // Deauth detection (subtype 12 = deauth)
  if (subtype == 12) {
    if (now - devices[idx].last_reset >= 1000) {
      devices[idx].deauth_count = 0;
      devices[idx].last_reset = now;
    }
    devices[idx].deauth_count++;
    
    if (devices[idx].deauth_count >= DEAUTH_THRESHOLD && !devices[idx].is_attacker) {
      devices[idx].is_attacker = true;
      deauth_alerts++;
      char mac_str[18];
      mac_to_str(mac, mac_str);
      Serial.printf("\n[!!!] DEAUTH ATTACK from %s (%d pkts/sec)\n\n", 
                    mac_str, devices[idx].deauth_count);
    }
  }
}

// Remove old devices
void cleanup_old_devices() {
  static uint32_t last_cleanup = 0;
  if (millis() - last_cleanup < 10000) return;
  last_cleanup = millis();
  
  for (int i = 0; i < device_count; i++) {
    if (millis() - devices[i].last_seen > 60000) {
      // Shift remaining devices
      for (int j = i; j < device_count - 1; j++) {
        devices[j] = devices[j + 1];
      }
      device_count--;
      i--;
    }
  }
}

// Display statistics
void show_statistics() {
  static uint32_t last_show = 0;
  if (millis() - last_show < 1000) return;
  last_show = millis();
  
  // Find strongest signal
  int best_idx = -1;
  for (int i = 0; i < device_count; i++) {
    if (best_idx == -1 || devices[i].rssi > devices[best_idx].rssi) {
      best_idx = i;
    }
  }
  
  // Count attackers
  int attacker_count = 0;
  for (int i = 0; i < device_count; i++) {
    if (devices[i].is_attacker) attacker_count++;
  }
  
  char mac_str[18] = "None";
  if (best_idx != -1) {
    mac_to_str(devices[best_idx].mac, mac_str);
  }
  
  Serial.printf("[STATS] Packets:%lu | Devices:%d | Attackers:%d | Strongest:%s (%d dBm)\n",
                total_packets, device_count, attacker_count, 
                mac_str, best_idx != -1 ? devices[best_idx].rssi : 0);
}

// Setup
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32 Wi-Fi Security Monitor ===\n");
  
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }
  
  // Initialize Wi-Fi in promiscuous mode
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(promiscuous_cb);
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE);
  
  // Max CPU speed
  setCpuFrequencyMhz(240);
  
  Serial.println("[READY] Monitoring channel 6");
  Serial.println("[INFO] Looking for Wi-Fi devices and deauth attacks...\n");
}

// Main loop
void loop() {
  // Process up to 50 packets per iteration
  for (int i = 0; i < 50 && queue_count > 0; i++) {
    process_packet();
  }
  
  cleanup_old_devices();
  show_statistics();
  
  delay(1);
}