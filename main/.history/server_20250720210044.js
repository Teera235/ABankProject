const express = require('express');
const bodyParser = require('body-parser');
const app = express();
const port = 3000;

// ใช้ body-parser เพื่ออ่านข้อมูล JSON
app.use(bodyParser.json());

// สร้าง route ที่รับ HTTP POST requests ที่ URL /api/data
app.post('/api/data', (req, res) => {
  const sensorData = req.body; // ข้อมูลที่ส่งมาจาก Arduino
  console.log('Received data:', sensorData);

  // ส่งคำตอบกลับไปที่ Arduino
  res.status(200).json({
    message: 'Data received successfully',
    receivedData: sensorData
  });
});

// เซิร์ฟเวอร์จะฟังที่พอร์ต 3000
app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
