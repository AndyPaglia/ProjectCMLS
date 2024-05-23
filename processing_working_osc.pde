import controlP5.*;
import oscP5.*;
import netP5.*;

ControlP5 cp5;
DropdownList dropdown;
PFont f;
PFont knobFont;
PFont font1;

String notearray[] = {"C ", "C#", "D ","D#", "E ", "F ","F#", "G ", "G#", "A ", "A#", "B "};
String currentNote = "C 3";

int valueX, valueY;

OscP5 oscP5;
NetAddress superColliderAddr; // Indirizzo di rete di SuperCollider
int ellipseX;
int ellipseY;
int diameter = 30; // Diametro del pallino
int rectX, rectY, rectWidth, rectHeight; // Rettangolo di confine
int rectX1, rectY1,rectWidth1, rectHeight1;  

void setup() {
  size(900, 600); // Imposta la dimensione della finestra
  smooth();
  f = createFont("Arial", 13, true); // Crea un font Arial in grassetto
  knobFont = createFont("Arial", 16, true); // Crea un font Arial più grande per i testi dei knob

  cp5 = new ControlP5(this);

  // Definisci le dimensioni e la posizione del rettangolo di confine
  rectWidth = 350;
  rectHeight = 350;
  rectX = width - rectWidth - 50; 
  rectY = (height - rectHeight) -50;
  
  // Definisci le dimensione del rettangolo di testo 
  rectWidth1 = 100;
  rectHeight1 = 100;
  rectX1 = rectX + rectWidth/2 - rectWidth1/2; 
  rectY1 = (height - rectHeight1)- 460;

  // Aggiungi i knob 
  cp5.addKnob("Unison", 1, 5, 1, 50, 50, 100).setDragDirection(Knob.VERTICAL).setNumberOfTickMarks(4).snapToTickMarks(true).setTickMarkLength(4)
     .setColorForeground(color(66,126,158)) // Colore del bordo
     .setColorBackground(color(51,65,71)) // Colore di sfondo
     .setColorActive(color(43,169,237)) // Colore quando è attiva
     .getCaptionLabel().setFont(knobFont).setColor(color(0)); // Imposta il font e il colore del testo in nero

  cp5.addKnob("Detune", 0.0, 1.0, 0.5, 250, 50, 100).setDragDirection(Knob.VERTICAL)
     .setColorForeground(color(66,126,158)) // Colore del bordo
     .setColorBackground(color(51,65,71)) // Colore di sfondo
     .setColorActive(color(43,169,237)) // Colore quando è attiva
     .getCaptionLabel().setFont(knobFont).setColor(color(0)); // Imposta il font e il colore del testo in nero
  
  cp5.addKnob("Volume", 0.0, 1.0, 0.5,140, 170, 120).setDragDirection(Knob.VERTICAL)
     .setColorForeground(color(66,126,158)) // Colore del bordo
     .setColorBackground(color(51,65,71)) // Colore di sfondo
     .setColorActive(color(43,169,237)) // Colore quando è attiva
     .getCaptionLabel().setFont(knobFont).setColor(color(0)); // Imposta il font e il colore del testo in nero
  
  // Inizializza la connessione OSC con SuperCollider
  oscP5 = new OscP5(this, 8000);
  superColliderAddr = new NetAddress("127.0.0.1", 57120); // Indirizzo di SuperCollider
  
  // Crea un'istanza di CColor con i colori desiderati
  CColor cColor = new CColor();
  cColor.setBackground(color(177,163,245));
  cColor.setForeground(color(10, 10, 10)); // Colore del testo
  cColor.setActive(color(10, 10, 10));   // Colore attivo

   // Carica e imposta il font
   font1 = createFont("Arial",16 ); // Sostituisci "Arial" con il font desiderato e 24 con la dimensione del carattere


  // Aggiungi la dropdown 
  dropdown = cp5.addDropdownList("menu")
                 .setPosition(50, 350) // Posiziona la dropdown 
                 .setSize(350, 350) //  grandezza della dropdown
                 .setItemHeight(40)
                 .setBackgroundColor(color(37,233,237))
                 .setBarHeight(40)
                 .setColor(cColor)
                 .setFont(font1);
                 
                 

  // Aggiungi le opzioni al menu
  dropdown.addItem("Sinusoid", 1);
  dropdown.addItem("Impulse", 2);
  dropdown.addItem("Sawtooth", 3);
  dropdown.addItem("Square", 4);

  // Inizializza la posizione del pallino all'interno del rettangolo di confine
  ellipseX = rectX + rectWidth / 2;
  ellipseY = rectY + rectHeight / 2;
}

void draw() {
  background(245,203,239); // Sfondo 

  // Disegna il rettangolo
  fill(217,242,216);
  stroke(0);
  rect(rectX, rectY, rectWidth, rectHeight);
  
  // Disegna il rettangolo
  fill(217,242,216);
  stroke(0);
  rect(rectX1, rectY1, rectWidth1+25, rectHeight1);
  

  // Disegna il testo "Knob"
  fill(0); // Colore del testo 0 = nero 
  textFont(f, 20); // Specifica il font da utilizzare
  text("Knobs:", 20, 30); // Disegna il knob
  
   // Disegna il testo 
  fill(0); // Colore del testo 0 = nero 
  textFont(f, 50); // Specifica il font da utilizzare
  text(currentNote, rectX1+12, rectY1+70); // Disegna il knob
  
     // Disegna il testo 
  fill(0); // Colore del testo 0 = nero 
  textFont(f, 20); // Specifica il font da utilizzare
  text("Note: ", rectX1 -120, rectY1-6); // Disegna il knob
  
       // Disegna il testo 
  fill(0); // Colore del testo 0 = nero 
  textFont(f, 20); // Specifica il font da utilizzare
  text("Vowel: ", rectX1 -120, rectY1 + 150); // Disegna il knob

  // Disegna il pallino 
  fill(21,106,163);
  ellipse(ellipseX, ellipseY, diameter, diameter);
}

void controlEvent(ControlEvent theEvent) {
  if (theEvent.isController()) {
    print("Control event from: " + theEvent.getController().getName() + " | ");
    println("Value: " + int(theEvent.getController().getValue()*1000)/1000.0);
    
    // Invia i valori dei knob tramite OSC a SuperCollider
    OscMessage msg;
    // Gestisci gli eventi della dropdown
    if (theEvent.getController().getName().equals("menu")) {
      msg = new OscMessage("/dropdown"); // Cambia il nome del messaggio OSC
      msg.add(int(theEvent.getController().getValue())); // Aggiungi il valore selezionato dalla dropdown
      oscP5.send(msg, superColliderAddr);
    }
    // Gestisci gli eventi dei knob
    else {
      // Valore intero per knob1
      if (theEvent.getController().getName().equals("Unison")) {
        println("in knob 1");
        msg = new OscMessage("/Unison"); // Cambia il nome del messaggio OSC
        int knobval = int(theEvent.getController().getValue());
        for (int i = 0; i < 5; i++) {
        if(i < knobval){
        msg.add(int(1));
          } else msg.add(int(0));
        }
        if(knobval == 1) msg.add(0);
        else {
          float val = ((180.0/(knobval-1))/90.0);
          float pan = -1.0;
          for(int j = 0; j < knobval; j++) {
            msg.add(pan);
            pan += val;
          }
        }
        for (int k = knobval; k < 5; k++) msg.add(float(0));
        oscP5.send(msg, superColliderAddr);
        println("typetag: " + msg.typetag());
      }
      // Valori float per knob2 e knob3
      else if (theEvent.getController().getName().equals("Detune") || theEvent.getController().getName().equals("Volume")) {
        msg = new OscMessage("/"+theEvent.getController().getName()); // Cambia il nome del messaggio OSC
        msg.add(theEvent.getController().getValue());
        oscP5.send(msg, superColliderAddr);
      }
    }
    
  }
}

// Funzione per ricevere i valori tramite OSC
void oscEvent(OscMessage msg) {
  if (msg.checkTypetag("i")) { // Controlla che il messaggio contenga due interi
    if (msg.checkAddrPattern("/handMovement/x") == true) { // Controlla il messaggio
      valueX = msg.get(0).intValue(); // Estrae il primo valore (asse X)
    }
    if (msg.checkAddrPattern("/handMovement/y") == true) { // Controlla il messaggio
      valueY = msg.get(0).intValue(); // Estrae il primo valore (asse X)
    }
      // Mappa i valori X e Y sui limiti del canvas e li limita al rettangolo di confine
      ellipseX = constrain(int(map(valueX, 0, 126, rectX, rectX + rectWidth)), rectX + diameter / 2, rectX + rectWidth - diameter / 2);
      ellipseY = constrain(int(map(valueY, 0, 126, rectY, rectY + rectHeight)), rectY + diameter / 2, rectY + rectHeight - diameter / 2);
    }
  if(msg.checkAddrPattern("/MidiNote") == true){
    int val = msg.get(0).intValue();
    int octave = val/12 - 1;
    int note = val % 12;
    currentNote = notearray[note] + str(octave);
    
  }
}

// Funzione per mappare un float tra due range
float mapFloat(float value, float start1, float stop1, float start2, float stop2) {
  return start2 + (stop2 - stop2) * ((value - start1) / (stop1 - start1));
}
