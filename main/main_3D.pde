import processing.serial.*;
import java.util.ArrayList;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.awt.Desktop;
import controlP5.*;

// --- ตัวแปรสำหรับการสื่อสาร Serial และโมเดล 3D ---
Serial myPort;
PShape rocketModel;
float roll = 0, pitch = 0, yaw = 0;
boolean serialConnected = false;
boolean modelLoaded = false;
boolean isRecording = false;

// --- ตัวแปรสำหรับเก็บข้อมูลกราฟ ---
ArrayList<Float> rollHistory;
ArrayList<Float> pitchHistory;
ArrayList<Float> yawHistory;
final int HISTORY_LENGTH = 300;

// --- สีสำหรับกราฟแต่ละเส้น ---
color rollColor;
color pitchColor;
color yawColor;

// --- ตัวแปรสำหรับจำลองข้อมูล ---
boolean simulateData = false;
float simulationTime = 0;

// --- ตัวแปรสำหรับการบันทึก ---
FileWriter writer;
File outputFile;

// --- ตัวแปรสำหรับ GUI ---
ControlP5 cp5;

void setup() {
  size(1400, 700, P3D);
  
  // --- เริ่มต้นค่าสำหรับระบบกราฟ ---
  rollHistory = new ArrayList<Float>();
  pitchHistory = new ArrayList<Float>();
  yawHistory = new ArrayList<Float>();
  
  rollColor = color(255, 87, 87);
  pitchColor = color(87, 255, 150);
  yawColor = color(87, 150, 255);
  
  // --- ตั้งค่า GUI ---
  cp5 = new ControlP5(this);
  cp5.addButton("toggleSimulation")
     .setPosition(10, height-80)
     .setSize(100, 20)
     .setLabel("Toggle Simulation");
  cp5.addButton("reconnectSerial")
     .setPosition(120, height-80)
     .setSize(100, 20)
     .setLabel("Reconnect");
  cp5.addButton("toggleRecording")
     .setPosition(230, height-80)
     .setSize(100, 20)
     .setLabel("Record/Stop");
  cp5.addButton("clearData")
     .setPosition(340, height-80)
     .setSize(100, 20)
     .setLabel("Clear Data");
  
  // --- ตั้งค่า Serial Port ---
  setupSerial();
  
  // --- โหลดโมเดล 3D ---
  loadRocketModel();
  
  smooth(8);
}

void setupSerial() {
  println("Available Serial Ports:");
  String[] portList = Serial.list();
  for (int i = 0; i < portList.length; i++) {
    println("[" + i + "] " + portList[i]);
  }
  
  if (portList.length == 0) {
    println("No serial ports available - switching to simulation mode");
    simulateData = true;
    return;
  }
  
  String targetPort = portList[0]; // ใช้พอร์ตแรกเป็นค่าเริ่มต้น
  int retryCount = 3;
  while (retryCount > 0) {
    try {
      myPort = new Serial(this, targetPort, 115200);
      myPort.bufferUntil('\n');
      serialConnected = true;
      println("Serial connected to: " + targetPort);
      break;
    } catch (Exception e) {
      println("Cannot connect to " + targetPort + ": " + e.getMessage());
      retryCount--;
      if (retryCount == 0) {
        println("Switching to simulation mode...");
        simulateData = true;
        serialConnected = false;
      }
      delay(1000); // รอ 1 วินาทีก่อนลองใหม่
    }
  }
}

void loadRocketModel() {
  try {
    rocketModel = loadShape("C:/Users/teerathap/Downloads/CubeSat/CubeSat.obj");
    if (rocketModel != null) {
      rocketModel.scale(100);
      centerModel(rocketModel);
      modelLoaded = true;
      println("Model loaded successfully");
    } else {
      println("Model file not found - using default cube");
      createDefaultModel();
    }
  } catch (Exception e) {
    println("Error loading model: " + e.getMessage());
    createDefaultModel();
  }
}

void createDefaultModel() {
  rocketModel = createShape(BOX, 100, 100, 100);
  rocketModel.setFill(color(100, 150, 255));
  modelLoaded = true;
}

void draw() {
  background(30, 30, 45);
  
  if (simulateData) {
    simulateRocketData();
  }
  
  int modelViewWidth = width / 2;
  int dashboardWidth = width / 2;

  // --- แสดงโมเดล 3D ---
  pushMatrix();
  translate(modelViewWidth / 2, height / 2, 0);
  lights();
  rotateY(radians(-yaw));
  rotateX(radians(-pitch));
  rotateZ(radians(roll));
  if (modelLoaded) {
    shape(rocketModel);
  }
  drawAxes();
  popMatrix();

  // --- แสดง Dashboard ---
  pushMatrix();
  translate(modelViewWidth, 0);
  drawDashboard(dashboardWidth, height);
  popMatrix();
  
  // --- แสดงสถานะการเชื่อมต่อ ---
  drawConnectionStatus();
  
  // --- บันทึกข้อมูลถ้ากำลังบันทึก ---
  if (isRecording) {
    try {
      writer.append(nf(millis(), 10) + "," + roll + "," + pitch + "," + yaw + "\n");
    } catch (IOException e) {
      println("Error writing to file: " + e.getMessage());
    }
  }
}

void drawAxes() {
  strokeWeight(3);
  stroke(255, 0, 0);
  line(0, 0, 0, 80, 0, 0);
  stroke(0, 255, 0);
  line(0, 0, 0, 0, 80, 0);
  stroke(0, 0, 255);
  line(0, 0, 0, 0, 0, 80);
  strokeWeight(1);
}

void drawConnectionStatus() {
  fill(255);
  textSize(12);
  String status = serialConnected ? "● Serial Connected" : simulateData ? "● Simulation Mode" : "● Disconnected";
  fill(serialConnected ? color(0, 255, 0) : simulateData ? color(255, 255, 0) : color(255, 0, 0));
  text(status, 10, height - 40);
  fill(255);
  text("Model: " + (modelLoaded ? "Loaded" : "Not Found"), 10, height - 20);
}

void simulateRocketData() {
  simulationTime += 0.05;
  float noise = random(-5, 5);
  roll = constrain(sin(simulationTime) * 45 + noise, -180, 180);
  pitch = constrain(cos(simulationTime * 0.7) * 30 + noise, -180, 180);
  yaw = constrain(sin(simulationTime * 0.3) * 60 + noise, -180, 180);
  addDataToHistory(roll, pitch, yaw);
}

void addDataToHistory(float r, float p, float y) {
  rollHistory.add(r);
  pitchHistory.add(p);
  yawHistory.add(y);
  if (rollHistory.size() > HISTORY_LENGTH) {
    rollHistory.remove(0);
    pitchHistory.remove(0);
    yawHistory.remove(0);
  }
}

void serialEvent(Serial p) {
  if (!serialConnected) return;
  try {
    String data = p.readStringUntil('\n');
    if (data != null) {
      data = trim(data);
      if (data.length() > 0) {
        String[] angles = split(data, ',');
        if (angles.length == 3) {
          float newRoll = constrain(float(angles[0]), -180, 180);
          float newPitch = constrain(float(angles[1]), -180, 180);
          float newYaw = constrain(float(angles[2]), -180, 180);
          if (!Float.isNaN(newRoll) && !Float.isNaN(newPitch) && !Float.isNaN(newYaw)) {
            roll = newRoll;
            pitch = newPitch;
            yaw = newYaw;
            addDataToHistory(roll, pitch, yaw);
          }
        }
      }
    }
  } catch (Exception e) {
    println("Serial error: " + e.getMessage());
    serialConnected = false;
    simulateData = true;
  }
}

void drawDashboard(int w, int h) {
  // --- แสดงค่าตัวเลข ---
  fill(240);
  textSize(22);
  text("REAL-TIME DATA", 30, 50);
  fill(rollColor);
  text("Roll : " + nf(roll, 1, 2) + "°", 30, 100);
  fill(pitchColor);
  text("Pitch: " + nf(pitch, 1, 2) + "°", 30, 140);
  fill(yawColor);
  text("Yaw  : " + nf(yaw, 1, 2) + "°", 30, 180);

  // --- วาดกราฟ ---
  int graphX = 30;
  int graphY = 240;
  int graphW = w - 60;
  int graphH = h - 280;
  
  noStroke();
  fill(20, 20, 30);
  rect(graphX, graphY, graphW, graphH, 5);
  
  // วาดแกนและกริด
  stroke(100);
  line(graphX, graphY + graphH/2, graphX + graphW, graphY + graphH/2); // แกน X
  line(graphX, graphY, graphX, graphY + graphH); // แกน Y
  fill(200);
  textSize(10);
  text("0°", graphX - 20, graphY + graphH/2 + 5);
  text("180°", graphX - 20, graphY + 5);
  text("-180°", graphX - 20, graphY + graphH - 5);
  
  if (rollHistory.size() > 1) {
    drawGraphLine(rollHistory, rollColor, -180, 180, graphX, graphY, graphW, graphH);
    drawGraphLine(pitchHistory, pitchColor, -180, 180, graphX, graphY, graphW, graphH);
    drawGraphLine(yawHistory, yawColor, -180, 180, graphX, graphY, graphW, graphH);
  }
  
  // --- วาดคำอธิบายสี ---
  int legendY = 100;
  fill(rollColor);
  rect(w - 150, legendY, 20, 10);
  fill(240);
  text("Roll", w - 120, legendY + 10);
  fill(pitchColor);
  rect(w - 150, legendY + 40, 20, 10);
  fill(240);
  text("Pitch", w - 120, legendY + 50);
  fill(yawColor);
  rect(w - 150, legendY + 80, 20, 10);
  fill(240);
  text("Yaw", w - 120, legendY + 90);
  
  fill(200);
  textSize(12);
  text("Data points: " + rollHistory.size() + "/" + HISTORY_LENGTH, 30, graphY + graphH + 20);
}

void drawGraphLine(ArrayList<Float> data, color c, float yMin, float yMax, float x, float y, float w, float h) {
  if (data.size() < 2) return;
  stroke(c);
  strokeWeight(2);
  noFill();
  beginShape();
  for (int i = 0; i < data.size(); i++) {
    float val = data.get(i);
    float graphX = map(i, 0, HISTORY_LENGTH - 1, x, x + w);
    float graphY = map(val, yMin, yMax, y + h, y);
    vertex(graphX, graphY);
  }
  endShape();
}

void centerModel(PShape model) {
  if (model == null) return;
  try {
    float minX = Float.MAX_VALUE, minY = Float.MAX_VALUE, minZ = Float.MAX_VALUE;
    float maxX = -Float.MAX_VALUE, maxY = -Float.MAX_VALUE, maxZ = -Float.MAX_VALUE;
    for (int i = 0; i < model.getChildCount(); i++) {
      PShape child = model.getChild(i);
      if (child != null) {
        for (int j = 0; j < child.getVertexCount(); j++) {
          PVector v = child.getVertex(j);
          if (v != null) {
            minX = min(minX, v.x);
            minY = min(minY, v.y);
            minZ = min(minZ, v.z);
            maxX = max(maxX, v.x);
            maxY = max(maxY, v.y);
            maxZ = max(maxZ, v.z);
          }
        }
      }
    }
    float centerX = (minX + maxX) / 2.0;
    float centerY = (minY + maxY) / 2.0;
    float centerZ = (minZ + maxZ) / 2.0;
    model.translate(-centerX, -centerY, -centerZ);
  } catch (Exception e) {
    println("Error centering model: " + e.getMessage());
  }
}

// --- ฟังก์ชันควบคุม GUI ---
void toggleSimulation() {
  simulateData = !simulateData;
  println("Simulation mode: " + (simulateData ? "ON" : "OFF"));
}

void reconnectSerial() {
  println("Attempting to reconnect...");
  setupSerial();
}

void toggleRecording() {
  if (!isRecording) {
    try {
      outputFile = new File("rocket_data_" + nf(millis(), 10) + ".csv");
      writer = new FileWriter(outputFile, true);
      writer.append("Timestamp,Roll,Pitch,Yaw\n");
      isRecording = true;
      println("เริ่มบันทึกข้อมูล...");
    } catch (IOException e) {
      println("ข้อผิดพลาดในการเริ่มบันทึก: " + e.getMessage());
    }
  } else {
    try {
      isRecording = false;
      writer.close();
      println("หยุดบันทึกและบันทึกข้อมูลสำเร็จ");
      if (Desktop.isDesktopSupported()) {
        Desktop.getDesktop().open(outputFile);
      }
    } catch (IOException e) {
      println("ข้อผิดพลาดในการหยุดบันทึก: " + e.getMessage());
    }
  }
}

void clearData() {
  rollHistory.clear();
  pitchHistory.clear();
  yawHistory.clear();
  println("Data cleared");
}

void keyPressed() {
  if (key == 's' || key == 'S') toggleSimulation();
  if (key == 'r' || key == 'R') reconnectSerial();
  if (key == 'w' || key == 'W') toggleRecording();
  if (key == 'c' || key == 'C') clearData();
}

void stop() {
  if (isRecording && writer != null) {
    try {
      writer.close();
      println("บันทึกข้อมูลและปิดไฟล์สำเร็จ");
    } catch (IOException e) {
      println("ข้อผิดพลาดในการปิดไฟล์: " + e.getMessage());
    }
  }
}
