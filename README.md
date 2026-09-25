# EV-Charging-Optimization-AI
AI-based EV Charging Station Optimization System
# 🚗 EV Charging Optimization with AI

## 📌 Project Overview

This project aims to optimize EV charging station usage using Artificial Intelligence.

The system recommends the best charging station based on:
- Battery Level
- Distance
- Traffic
- Queue Length
- Availability of Charging Slots

---

## 🎯 Objectives

- Reduce waiting time
- Optimize charger allocation
- Improve charging station utilization
- Predict charging demand using AI

## 🛠 Technologies
- IOT
- MongoDB
- Express.js
- React.js
- Node.js
- Artificial Intelligence
- Git & GitHub

## 📂 Project Documentation

- Problem Statement
- ER Diagram
- Database Design
- AI Model
- API Design
- Daily Progress
## 🚀 Project Status

🟢 Documentation Phase

## 🧩 Project Structure

```
EV-Charging-Optimization-AI/
├── client/          # React frontend (Vite)
├── server/          # Node.js + Express backend
│   └── src/
│       ├── config/       # DB connection
│       ├── controllers/  # Route logic
│       ├── models/       # Mongoose schemas
│       └── routes/       # API routes
├── Hardware (Electronic)/  # ESP32 / IoT firmware
├── docs/            # Project documentation
├── diagrams/        # Architecture & ER diagrams
└── research paper/  # Reference papers
```

## ▶️ Getting Started

### Backend (server)
```bash
cd server
cp .env.example .env   # then set your MongoDB URI
npm install
npm run dev             # starts on http://localhost:5000
```

### Frontend (client)
```bash
cd client
npm install
npm run dev             # starts on http://localhost:3000
```

The client's dev server proxies `/api` requests to the backend (see `client/vite.config.js`).
