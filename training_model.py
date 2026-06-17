import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeRegressor
from sklearn.metrics import mean_absolute_error, r2_score
from sklearn import tree

# Load dataset
df = pd.read_csv("housing.csv")


# Remove missing rows
df = df.dropna()

# Input features
X = df[
    [
        "median_income",
        "latitude",
        "longitude",
        "housing_median_age"
    ]
]

# Target
y = df["median_house_value"]

# Split dataset
X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.2,
    random_state=42
)

# Small model for ESP32
model = DecisionTreeRegressor(
    max_depth=4,
    random_state=42
)

# Train
model.fit(X_train, y_train)

# Predict
pred = model.predict(X_test)

# Metrics
print("\nResults")
print("-" * 40)
print("MAE =", mean_absolute_error(y_test, pred))
print("R2  =", r2_score(y_test, pred))

print("\nFeature Importance")
print("-" * 40)

for name, imp in zip(X.columns, model.feature_importances_):
    print(f"{name}: {imp:.4f}")

print("\nDecision Tree Rules")
print("-" * 40)

rules = tree.export_text(
    model,
    feature_names=list(X.columns)
)

print(rules)