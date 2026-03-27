# Women-Safety-IOT-Band
ESP32-based IoT safety band with heart rate &amp; SpO₂ monitoring, GPS tracking, and Telegram alert system with ML-based panic detection.
A wearable IoT-based safety device designed to provide immediate help during emergency situations. The system monitors heart rate and SpO₂ continuously and sends an alert with live location when danger is detected or when the user presses a panic button.

Features :
Real-time GPS location tracking
Emergency alert via Telegram
Heart rate & SpO₂ monitoring (MAX30102)
Panic detection using machine learning (Logistic Regression)
Manual SOS button trigger
OLED display for live data
IoT-based system using ESP32

Tech : 
Microcontroller: ESP32
Sensors: MAX30102 (Heart Rate & SpO₂), NEO-6M (GPS)
Communication: Wi-Fi, Telegram Bot API
Programming: Arduino IDE (Embedded C++)
Machine Learning: Logistic Regression (Binary Classification)

How It Works : 
	1.	The system continuously monitors heart rate and SpO₂.
	2.	If abnormal heart rate is detected → panic condition is identified.
	3.	OR user presses the emergency button.
	4.	GPS fetches real-time location.
	5.	Alert + Google Maps link is sent via Telegram.
	6.	OLED displays live health data.
  
  Machine Learning (Basic Idea) :
	•	Uses binary classification
	•	Input: Heart rate patterns
	•	Output:
	•	Normal
	•	Panic
	•	Helps reduce false alarms (e.g., exercise vs panic)
