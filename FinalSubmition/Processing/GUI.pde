import controlP5.*; // importar bibliotecas
import processing.serial.*;
//import grafica.*;  // Import Grafica library

ControlP5 cp5; // objeto para controlador de interface gráfica
Serial myPort; // objeto comunicaçao serial
DropdownList portList;
String[] portas_atuais; // Lista de portas atuais

color backgroundcolor = #D8E1E9;
color textColor = #000000;
color buttonColor = #3B5368;
color buttonHoverColor = #2C3E4E;

int originalHeight = 350;
int originalWidth = 800;

int centerWidth = originalWidth/2;

void setup() 
{
  size(800, 350);
  smooth(8);

  //--------------------------------Create Object---------------------------------

  cp5 = new ControlP5(this); // cria controlador de interface gráfica

  //-----------------------------------------------------------------------------

  //------------------------------------FONTS------------------------------------
  
  // change the default font to Times New Roman
  pixelDensity(displayDensity());
  PFont b = createFont("Times New Roman", 18, true);
  ControlFont Ourfont = new ControlFont(b);
  cp5.setFont(Ourfont);

  //-----------------------------------------------------------------------------
  
  // Verifica se há portas seriais disponíveis
  portas_atuais = Serial.list();// retorna uma lista das portas serials disponíveis no sistema
    
  if (portas_atuais.length == 0) {
    println("Nenhuma porta serial conectada");
    noLoop(); // Para o draw()
    exit(); // termina o programa
  }
  
  println("Portas disponíveis:");
  printArray(portas_atuais); // lista todas as portas serial disponíveis no terminal
  
  // Dropdown para selecionar a porta serial
  portList = cp5.addDropdownList("Serial Ports")
             .setPosition(centerWidth - 150, 50)
             .setSize(300, 200)
             .setItemHeight(50)
             .setBarHeight(50)
             .setColorBackground(buttonColor)
             .setColorActive(buttonHoverColor)
             .setLabel("Select Serial Port"); 

  // Adiciona as portas seriais disponíveis ao menu dropdown
  for (int i = 0; i < portas_atuais.length; i++) {
    portList.addItem(portas_atuais[i], i); // associa o nome do item a um índice
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
}

void draw() 
{
  background(backgroundcolor); // define a cor de background
}

void controlEvent(ControlEvent e)  // cria evento
{
  if (e.isFrom(portList)) // verifica se alguma porta foi selecionada
  {
    int n_porta = int(e.getValue()); // retorna o índice da porta serial selecionada
    String portName = Serial.list()[n_porta]; // retorna o nome da porta selecionada
    println("Selecionada a porta: " + portName);
    
    if (myPort != null) // Verifica se alguma porta está previamente conectada
    { 
      myPort.stop(); // Se alterar a porta no dropdown, a porta anterior ativa é fechada
    }
      myPort = new Serial(this, portName, 115200); // cria nova comunicação serial com a porta selecionada
  }
  
  if (e.getName().equals("abrir") || e.getName().equals("fechar")) 
  { 
    if (myPort != null) {
      if (e.getName().equals("abrir")) {
        myPort.write("A\n");
        println("Comando enviado: A (Abrir)");
      } 
      else if (e.getName().equals("fechar")) {
        myPort.write("F\n");
        println("Comando enviado: F (Fechar)");
      } 
    } 
    else {
      // Simulate commands when no serial port is connected
      String simulatedMsg = "Simulated command (no serial port): " + e.getName();
      println(simulatedMsg);
    }
  }
} 