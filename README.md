# 🎙️ Voice-Controlled RGB LED via Edge AI (TinyML)

**Module:** CASA0018 - Connected Environments  
**Author:** Jiahua He

[![Arduino](https://img.shields.io/badge/Platform-Arduino-blue.svg)](https://www.arduino.cc/)
[![Edge Impulse](https://img.shields.io/badge/TinyML-Edge_Impulse-lightgrey.svg)](https://www.edgeimpulse.com/)

## 📖 Project Overview
This project explores the deployment of Machine Learning on resource-constrained edge devices (TinyML). It implements a fully local, on-device voice recognition system using an **Arduino Nano 33 BLE Sense**. The system listens to the environment, processes the audio data through a TensorFlow Lite model, and controls an onboard RGB LED based on 6 classified states: `red`, `green`, `blue`, `yellow`, `noise`, and `unknown`.

**Demo Video:** [https://youtu.be/KHVB5II-IP0]

**🧠 Model Evolution & Critical Reflection**
A core objective of this project was to understand how model architecture impacts edge deployment. Throughout the project, three model iterations were tested:

V1 (Basic Version): An initial basic model with some initial issues, consisting of only three colors - red, blue, and green, and without including any noise data.

V2 (Custom 1D CNN - Deployed Version): A lightweight CNN trained from scratch on MFCC features. It achieved a 92.6% test accuracy, requiring only 12.6K RAM and 3ms inference time. This model provided the perfect "Model-Data Fit" for the limited dataset (approx. 55 samples per class).

V3 (MobileNetV2 Transfer Learning): An experimental iteration to test if a pre-trained architecture could improve robustness. However, accuracy dropped to 74.6%, with a significant increase in 'Uncertain' classifications, inferencing latency and memory overhead.The inferencing latency was 644 ms, whilst the peak SRAM memory footprint was 168.5 KB. The inference time and RAM usage are much higher than those of the V2 model.

Key Takeaway: Bigger is not always better in TinyML. Deploying massive pre-trained models on small, local datasets can lead to confidence dilution. The V2 lightweight CNN was selected as the final deployed model due to its optimal balance of accuracy and computational efficiency.

**⚙️ Hardware & Software Requirements**
Hardware: Arduino Nano 33 BLE Sense (Microphone and onboard Common-Anode RGB LED utilized).

Software: Arduino IDE

ML Pipeline: Edge Impulse Studio.

**🚀 How to Run the Project**
Clone the repository:

Bash
git clone: https://github.com/virtuosa0714/casa0018-Project 

Open the Sketch:
Open src/nano_ble33_voice_control/nano_ble33_voice_control.ino in your Arduino IDE.

Install the Inferencing Library:
Download the exported .zip library from Edge Impulse (or use the one provided if uploaded), and install it via Sketch -> Include Library -> Add .ZIP Library... in the Arduino IDE.

Flash to Board:
Connect your Arduino Nano 33 BLE Sense, select the correct COM port, and click Upload. (Note: Initial compilation of the ML library may take up to 10-15 minutes).

Test:
Open the Serial Monitor at 115200 baud rate. Say "Red", "Green", "Blue", or "Yellow" clearly near the microphone to control the LED.

**🛠️ Hardware Implementation Note**
The onboard RGB LED of the Arduino Nano 33 BLE Sense uses a Common-Anode configuration. Therefore, the GPIO logic is active-low (LOW turns the LED ON, HIGH turns it OFF). Yellow is achieved by simultaneously pulling both the Red and Green pins LOW. A confidence threshold of 0.8 is hardcoded to prevent mis-triggers from environmental noise.
