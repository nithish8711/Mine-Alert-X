# ⛏️ Mine Alert – X

**Mine Alert – X** is an automated **coal mine safety alert system** that leverages **ESP-NOW wireless communication** and sensor integration to ensure worker safety in underground mining environments.  

The system continuously monitors environmental parameters such as **temperature, humidity, toxic gases (CO, CH4), and oxygen levels**. Alerts are instantly transmitted to the surface monitoring unit and nearby workers, enabling **real-time safety response**.

---

## 🚀 Key Features

- **Wireless Communication (ESP-NOW):** No dependency on Wi-Fi or internet inside mines  
- **Multi-Sensor Integration:** Monitors CO, CH4, O₂, temperature, and humidity  
- **Real-Time Alerts:** Immediate notifications when safety thresholds are breached  
- **Low-Power Operation:** ESP32 nodes optimized for long-term underground use  
- **Mesh Support:** Multiple sensor nodes can communicate with a central receiver  
- **Scalable Design:** Supports expansion to multiple mines and monitoring stations  

---

## 📂 Project Structure

- `sender_node/` → ESP32 code for **sensor nodes** inside the mine  
- `receiver_node/` → ESP32 code for **receiver unit** at the surface  
- `alert_system/` → Optional integration with buzzer, LEDs, or Firebase for remote monitoring  

---

## ⚙️ Hardware Setup

- **Sensor Node (Underground Worker Safety Unit)**  
  - ESP32  
  - DHT11/DHT22 (Temperature & Humidity)  
  - MQ-7 (Carbon Monoxide)  
  - MQ-4 (Methane)  
  - O₂ Sensor (Electrochemical)  
  - Power Supply (Battery Pack)  

- **Receiver Node (Surface Monitoring Unit)**  
  - ESP32  
  - Connected to alarm system / Raspberry Pi for data logging  

---

## 🔧 How to Use

1. Flash `sender_node` code onto ESP32 sensor nodes.  
2. Flash `receiver_node` code onto ESP32 receiver.  
3. Power on the sensor nodes inside the mine.  
4. Start the receiver at the surface monitoring station.  
5. View real-time sensor data and alerts via serial monitor or Firebase integration.  

---

## 📡 Communication Flow

1. Sensor node reads data → sends via **ESP-NOW**  
2. Receiver node collects data → triggers **alerts** if thresholds exceeded  
3. Optionally, receiver uploads data to **Firebase / Web Dashboard**  

---

## 📌 Future Enhancements

- Cloud dashboard for remote monitoring  
- Worker wearable device with vibration alerts  
- Integration with government mine safety systems  
- AI-based anomaly detection for predictive hazard alerts  

---

## 🖼 System Diagram (Conceptual)

```mermaid
graph TD
    A[Sensor Node 1 - ESP32 + Sensors] --> C[Receiver Node - ESP32]
    B[Sensor Node 2 - ESP32 + Sensors] --> C
    C --> D[Alert System - Buzzer/LED]
    C --> E[Firebase / Web Dashboard]
