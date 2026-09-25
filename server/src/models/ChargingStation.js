const mongoose = require("mongoose");

const chargingStationSchema = new mongoose.Schema(
  {
    name: { type: String, required: true },
    location: {
      lat: { type: Number, required: true },
      lng: { type: Number, required: true },
    },
    totalSlots: { type: Number, required: true, default: 1 },
    availableSlots: { type: Number, required: true, default: 1 },
    queueLength: { type: Number, default: 0 },
    trafficLevel: {
      type: String,
      enum: ["low", "medium", "high"],
      default: "low",
    },
    status: {
      type: String,
      enum: ["active", "inactive", "maintenance"],
      default: "active",
    },
  },
  { timestamps: true }
);

module.exports = mongoose.model("ChargingStation", chargingStationSchema);
