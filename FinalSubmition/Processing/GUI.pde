import controlP5.*;
import processing.serial.*;
import grafica.*;

ControlP5 cp5;
Serial myPort;
DropdownList portList;
String[] portas_atuais;

color backgroundcolor = #D8E1E9;
color textColor = #000000;
color buttonColor = #3B5368;
color buttonHoverColor = #2C3E4E;

int originalHeight = 700;
int originalWidth = 1000;
int centerWidth = originalWidth / 2;

float magZ;
GPlot plot;
ArrayList<GPoint> points;
float startTime;
float time;

void setup() {
  size(1000, 700);
  smooth();

  cp5 = new ControlP5(this);

  // Setup plot
  plot = new GPlot(this);
  plot.setPos(centerWidth - 325, 350);
  plot.setOuterDim(650, 300);
  plot.getXAxis().setAxisLabelText("Time (s)");
  plot.getYAxis().setAxisLabelText("Value");
  plot.setTitleText("Live Serial Plot");
  plot.setLineColor(color(50, 100, 250));
  plot.setPointColor(color(200, 50, 50));

  // Font setup
  pixelDensity(displayDensity());
  PFont b = createFont("Times New Roman", 18, true);
  ControlFont Ourfont = new ControlFont(b);
  cp5.setFont(Ourfont);

  // Serial ports
  portas_atuais = Serial.list();
  if (portas_atuais.length == 0) {
    println("Nenhuma porta serial conectada");
    exit();
  }

  println("Portas disponíveis:");
  printArray(portas_atuais);

  portList = cp5.addDropdownList("Serial Ports")
    .setPosition(centerWidth - 150, 50)
    .setSize(300, 200)
    .setItemHeight(50)
    .setBarHeight(50)
    .setColorBackground(buttonColor)
    .setColorActive(buttonHoverColor)
    .setLabel("Select Serial Port"); 

  for (int i = 0; i < portas_atuais.length; i++) {
    portList.addItem(portas_atuais[i], i);
  }

  portList.close();

  cp5.addButton("abrir")
    .setPosition(centerWidth - 175, 250)
    .setSize(150, 50)
    .setLabel("OPEN")
    .setColorBackground(buttonColor);

  cp5.addButton("fechar")
    .setPosition(centerWidth + 25, 250)
    .setSize(150, 50)
    .setLabel("CLOSE")
    .setColorBackground(buttonColor);

  startTime = millis();
}

void draw() {
  background(backgroundcolor);
  drawGraph();  
  
  plot.beginDraw();
  plot.drawBox();
  plot.drawXAxis();
  plot.drawYAxis();
  plot.drawTitle();
  plot.drawPoints();
  plot.endDraw();
}

void controlEvent(ControlEvent e) {
  if (e.isFrom(portList)) {
    int n_porta = int(e.getValue());
    String portName = Serial.list()[n_porta];
    println("Selecionada a porta: " + portName);

    if (myPort != null) {
      myPort.stop();
    }
    myPort = new Serial(this, portName, 115200);
  }

  if (e.getName().equals("abrir") || e.getName().equals("fechar")) {
    if (myPort != null) {
      if (e.getName().equals("abrir")) {
        myPort.write("A\n");
        println("Comando enviado: A (Abrir)");
      } else if (e.getName().equals("fechar")) {
        myPort.write("F\n");
        println("Comando enviado: F (Fechar)");
      }
    } else {
      println("Simulado (sem porta): " + e.getName());
    }
  }
}

void serialEvent(Serial myPort) {
  String data = myPort.readStringUntil('\n');
  if (data != null) {
    data = trim(data);
    try {
      float receivedNumber = float(data);
      magZ = receivedNumber;
      println("Dados recebidos: " + magZ);

      float elapsedSeconds = (millis() - startTime) / 1000.0;
      time = elapsedSeconds;
    } catch (Exception e) {
      println("Erro ao converter: " + data);
    }
  }
}

GPointsArray drawGraph()
{
  points = generateform(magZ,time);
  GPointsArray gPoints = new GPointsArray();

  // Convert points to GPointsArray and set to plot
  for (GPoint point : points) {
    gPoints.add(point);
  }
  plot.setPoints(gPoints);

  return gPoints;

}


ArrayList<GPoint> generateform(float magZ, float time) 
{
  ArrayList<GPoint> wavePoints = new ArrayList<GPoint>();
  
  wavePoints.add(new GPoint(magZ, time));
  
  return wavePoints;
}