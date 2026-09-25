const ChargingSession = require("../models/ChargingSession");

// @desc  ESP32 posts live sensor data here every few seconds
// @route POST /api/sessions/telemetry
exports.receiveTelemetry = async (req, res) => {
  try {
    const { stationId, portId, uid, current, voltage, temperature, status } = req.body;

    if (!stationId || !portId || current === undefined || voltage === undefined) {
      return res.status(400).json({ message: "Missing required telemetry fields" });
    }

    const session = await ChargingSession.create({
      stationId,
      portId,
      uid,
      current,
      voltage,
      temperature,
      status,
    });

    res.status(201).json(session);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};

// @desc  Get latest reading per port for a station (used by the dashboard)
// @route GET /api/sessions/latest/:stationId
exports.getLatestByStation = async (req, res) => {
  try {
    const sessions = await ChargingSession.find({ stationId: req.params.stationId })
      .sort({ createdAt: -1 })
      .limit(2);
    res.status(200).json(sessions);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};

// @desc  Get full session history for a station (for charts / AI training data)
// @route GET /api/sessions/:stationId
exports.getSessionHistory = async (req, res) => {
  try {
    const sessions = await ChargingSession.find({ stationId: req.params.stationId }).sort({
      createdAt: -1,
    });
    res.status(200).json(sessions);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};
