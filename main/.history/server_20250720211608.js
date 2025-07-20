const express = require('express');
const bodyParser = require('body-parser');
const app = express();
const port = 3000;

app.use(bodyParser.json());

app.post('/api/data', (req, res) => {
  const sensorData = req.body; // ข้อมูลที่ส่งมาจาก Arduino
  console.log('Received data:', sensorData);

  res.status(200).json({
    message: 'Data received successfully',
    receivedData: sensorData
  });
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
const char *ssid = "Ruchikan_2.4GHz";
const char *password = "0972905340";