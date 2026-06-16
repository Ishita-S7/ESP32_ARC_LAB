#include "iris_model.h"

float input[4];

Eloquent::ML::Port::RandomForest model;

String getFlower(int c) {
  if (c == 0) return "Setosa";
  if (c == 1) return "Versicolor";
  if (c == 2) return "Virginica";
  return "Unknown";
}

void setup() {
  Serial.begin(115200);
  Serial.println("Ready");
}

void loop() {
  if (Serial.available()) {

    String data = Serial.readStringUntil('\n');

    sscanf(data.c_str(), "%f,%f,%f,%f",
           &input[0], &input[1], &input[2], &input[3]);

    int result = model.predict(input);

    Serial.print("Prediction: ");
    Serial.println(getFlower(result));
  }
}