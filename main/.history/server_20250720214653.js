const express = require('express');
const app = express();
const port = 3000;

app.use(express.json());

app.post('/api/data', (req, res) => {
  const sensorData = req.body;
  console.log('Received data:', sensorData);

  res.status(200).json({
    message: 'Data received successfully',
    receivedData: sensorData
  });
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
