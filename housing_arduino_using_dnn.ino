#include "housing_fnn.h"


// ===============================
// ReLU Activation
// ===============================

float relu(float x)
{
  return (x > 0) ? x : 0;
}



// ===============================
// Dense Layer
// ===============================

void dense(
    float *input,
    float *output,
    float *weights,
    float *bias,
    int input_size,
    int output_size
)
{
  for(int i = 0; i < output_size; i++)
  {
    float sum = bias[i];

    for(int j = 0; j < input_size; j++)
    {
      sum += input[j] * weights[j * output_size + i];
    }

    output[i] = sum;
  }
}



// ===============================
// Take Input
// ===============================

float getInput(String name)
{
  Serial.print("Enter ");
  Serial.println(name);


  while(Serial.available() == 0)
  {
    delay(10);
  }


  float value = Serial.parseFloat();


  // Clear remaining characters (\n,\r)
  while(Serial.available())
  {
    Serial.read();
  }


  Serial.print("Received: ");
  Serial.println(value);


  return value;
}



// ===============================
// Setup
// ===============================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("Housing Price Prediction - DNN");
  Serial.println("--------------------------------");
}



// ===============================
// Loop
// ===============================

void loop()
{

  // Input order:
  // median_income
  // latitude
  // longitude
  // housing_median_age


  float input[4];


  input[0] = getInput("median_income");

  input[1] = getInput("latitude");

  input[2] = getInput("longitude");

  input[3] = getInput("housing_median_age");



  // ===============================
  // Normalize Input
  // ===============================

  float scaled[4];


  for(int i = 0; i < 4; i++)
  {
    scaled[i] =
      (input[i] - INPUT_MEAN[i])
      /
      INPUT_STD[i];
  }



  // ===============================
  // Layer 1
  // 4 -> 8
  // ===============================

  float layer1[8];


  dense(
    scaled,
    layer1,
    weight1,
    bias1,
    4,
    8
  );


  for(int i=0;i<8;i++)
  {
    layer1[i] = relu(layer1[i]);
  }



  // ===============================
  // Layer 2
  // 8 -> 4
  // ===============================

  float layer2[4];


  dense(
    layer1,
    layer2,
    weight2,
    bias2,
    8,
    4
  );


  for(int i=0;i<4;i++)
  {
    layer2[i] = relu(layer2[i]);
  }



  // ===============================
  // Output Layer
  // 4 -> 1
  // ===============================

  float output[1];


  dense(
    layer2,
    output,
    weight3,
    bias3,
    4,
    1
  );



  Serial.println("--------------------------------");

  Serial.print("Predicted House Value: ");

  Serial.println(output[0]);


  Serial.println("--------------------------------");

  Serial.println("Enter another house data set...");

  delay(1000);

}
