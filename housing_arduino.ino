// ===============================
// Decision Tree Model
// ===============================

float predictHouse(
    float median_income,
    float latitude,
    float longitude,
    float housing_median_age
)
{
    if (median_income <= 5.04)
    {
        if (median_income <= 3.11)
        {
            if (latitude <= 34.45)
            {
                if (longitude <= -118.31)
                    return 228473.53;
                else
                    return 142259.92;
            }
            else
            {
                if (longitude <= -121.69)
                    return 148316.27;
                else
                    return 93763.70;
            }
        }
        else
        {
            if (housing_median_age <= 47.50)
            {
                if (latitude <= 37.97)
                    return 210714.42;
                else
                    return 156169.21;
            }
            else
            {
                if (longitude <= -122.31)
                    return 352294.45;
                else
                    return 246453.40;
            }
        }
    }
    else
    {
        if (median_income <= 6.89)
        {
            if (housing_median_age <= 27.50)
            {
                if (median_income <= 5.72)
                    return 240795.24;
                else
                    return 290884.29;
            }
            else
            {
                if (median_income <= 5.76)
                    return 299432.07;
                else
                    return 372219.95;
            }
        }
        else
        {
            if (median_income <= 7.82)
            {
                if (housing_median_age <= 26.50)
                    return 342282.02;
                else
                    return 425338.56;
            }
            else
            {
                if (median_income <= 9.07)
                    return 436804.80;
                else
                    return 482674.76;
            }
        }
    }
}


// ===============================
// Setup
// ===============================

void setup()
{
    Serial.begin(115200);

    Serial.println("Housing Price Predictor");
    Serial.println("Enter:");
    Serial.println("median_income latitude longitude housing_median_age");
    Serial.println("Example:");
    Serial.println("8.3252 37.88 -122.23 41");
}


// ===============================
// Loop
// ===============================

void loop()
{
    if (Serial.available())
    {
        float median_income = Serial.parseFloat();
        float latitude = Serial.parseFloat();
        float longitude = Serial.parseFloat();
        float housing_median_age = Serial.parseFloat();


        float prediction = predictHouse(
            median_income,
            latitude,
            longitude,
            housing_median_age
        );


        Serial.print("Predicted House Price: $");
        Serial.println(prediction);

        Serial.println("---------------------");
    }
}