const mongoose = require("mongoose");

const chargingSessionSchema = new mongoose.Schema(
  {
    stationId: { type: String, required: true },
    portId: { type: Number, required: true }, // 1 or 2
    uid: { type: String, default: "" }, // RFID card UID
    current: { type: Number, required: true }, // amps
    voltage: { type: Number, required: true }, // volts
    temperature: { type: Number, required: true }, // celsius
    status: {
      type: String,
      enum: ["charging", "idle", "fault"],
      default: "idle",
    },
  },
  { timestamps: true }
);

module.exports = mongoose.model("ChargingSession", chargingSessionSchema);
