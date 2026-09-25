import { useEffect, useState } from "react";
import axios from "axios";

function App() {
  const [stations, setStations] = useState([]);
  const [error, setError] = useState(null);

  useEffect(() => {
    axios
      .get("/api/stations")
      .then((res) => setStations(res.data))
      .catch((err) => setError(err.message));
  }, []);

  return (
    <div style={{ fontFamily: "sans-serif", padding: "2rem" }}>
      <h1>EV Charging Optimization AI</h1>
      <p>Available Charging Stations</p>

      {error && <p style={{ color: "red" }}>Error: {error}</p>}

      <ul>
        {stations.map((station) => (
          <li key={station._id}>
            {station.name} — {station.availableSlots}/{station.totalSlots} slots free
          </li>
        ))}
      </ul>
    </div>
  );
}

export default App;
