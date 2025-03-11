import pandas as pd
import numpy as np
import tensorflow as tf
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

# Load dataset
df = pd.read_csv("EV_Battery_Data.csv")

# Extract features and targets
X = df[["Voltage (V)", "Current (A)", "Temperature (°C)"]].values
y_soc = df["SOC (%)"].values.reshape(-1, 1)
y_soh = df["SOH (%)"].values.reshape(-1, 1)

# Split data into training and testing sets
X_train, X_test, y_soc_train, y_soc_test, y_soh_train, y_soh_test = train_test_split(
    X, y_soc, y_soh, test_size=0.2, random_state=42
)

# Normalize features
scaler = StandardScaler()
X_train = scaler.fit_transform(X_train)
X_test = scaler.transform(X_test)

# Define the neural network model
def create_model():
    model = tf.keras.Sequential([
        tf.keras.layers.Dense(16, activation='relu', input_shape=(3,)),
        tf.keras.layers.Dense(8, activation='relu'),
        tf.keras.layers.Dense(2)  # Output: [SOC, SOH]
    ])
    model.compile(optimizer='adam', loss='mse')
    return model

model = create_model()

# Train the model
history = model.fit(X_train, np.hstack((y_soc_train, y_soh_train)), epochs=100, batch_size=10, validation_split=0.2)

# Evaluate the model
test_loss = model.evaluate(X_test, np.hstack((y_soc_test, y_soh_test)))
print(f'Test Loss: {test_loss}')

# Save model
model.save("battery_model.h5")

# Plot training history
plt.figure(figsize=(10,5))
plt.plot(history.history['loss'], label='Training Loss')
plt.plot(history.history['val_loss'], label='Validation Loss')
plt.xlabel('Epochs')
plt.ylabel('Loss')
plt.legend()
plt.title('Training and Validation Loss')
plt.show()

# Prediction function
def predict_battery_health(voltage, current, temperature):
    input_data = scaler.transform(np.array([[voltage, current, temperature]]))
    prediction = model.predict(input_data)
    print(f'Predicted SOC: {prediction[0][0]:.2f}%, Predicted SOH: {prediction[0][1]:.2f}%')
    return prediction
