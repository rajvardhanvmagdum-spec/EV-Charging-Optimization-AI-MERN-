const ChargingStation = require("../models/ChargingStation");

// @desc  Get all charging stations
// @route GET /api/stations
exports.getStations = async (req, res) => {
  try {
    const stations = await ChargingStation.find();
    res.status(200).json(stations);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};

// @desc  Create a new charging station
// @route POST /api/stations
exports.createStation = async (req, res) => {
  try {
    const station = await ChargingStation.create(req.body);
    res.status(201).json(station);
  } catch (error) {
    res.status(400).json({ message: error.message });
  }
};

// @desc  Get a single charging station by ID
// @route GET /api/stations/:id
exports.getStationById = async (req, res) => {
  try {
    const station = await ChargingStation.findById(req.params.id);
    if (!station) {
      return res.status(404).json({ message: "Station not found" });
    }
    res.status(200).json(station);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};

// @desc  Update a charging station
// @route PUT /api/stations/:id
exports.updateStation = async (req, res) => {
  try {
    const station = await ChargingStation.findByIdAndUpdate(
      req.params.id,
      req.body,
      { new: true, runValidators: true }
    );
    if (!station) {
      return res.status(404).json({ message: "Station not found" });
    }
    res.status(200).json(station);
  } catch (error) {
    res.status(400).json({ message: error.message });
  }
};

// @desc  Delete a charging station
// @route DELETE /api/stations/:id
exports.deleteStation = async (req, res) => {
  try {
    const station = await ChargingStation.findByIdAndDelete(req.params.id);
    if (!station) {
      return res.status(404).json({ message: "Station not found" });
    }
    res.status(200).json({ message: "Station deleted successfully" });
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};

// @desc  Get AI-recommended best station (placeholder logic)
// @route GET /api/stations/recommend
exports.recommendStation = async (req, res) => {
  try {
    // Placeholder: recommend the active station with most available slots
    // and shortest queue. Replace with real AI model integration later.
    const stations = await ChargingStation.find({ status: "active" });

    if (stations.length === 0) {
      return res.status(404).json({ message: "No active stations found" });
    }

    const best = stations.reduce((prev, curr) => {
      const prevScore = prev.availableSlots - prev.queueLength;
      const currScore = curr.availableSlots - curr.queueLength;
      return currScore > prevScore ? curr : prev;
    });

    res.status(200).json(best);
  } catch (error) {
    res.status(500).json({ message: error.message });
  }
};
