const express = require("express");
const router = express.Router();
const {
  getStations,
  createStation,
  getStationById,
  updateStation,
  deleteStation,
  recommendStation,
} = require("../controllers/chargingStationController");

router.get("/recommend", recommendStation);
router.get("/", getStations);
router.post("/", createStation);
router.get("/:id", getStationById);
router.put("/:id", updateStation);
router.delete("/:id", deleteStation);

module.exports = router;
