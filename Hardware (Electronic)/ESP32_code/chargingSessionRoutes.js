const express = require("express");
const router = express.Router();
const {
  receiveTelemetry,
  getLatestByStation,
  getSessionHistory,
} = require("../controllers/chargingSessionController");

router.post("/telemetry", receiveTelemetry);
router.get("/latest/:stationId", getLatestByStation);
router.get("/:stationId", getSessionHistory);

module.exports = router;
