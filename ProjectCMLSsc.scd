//setup
s.boot;
//CONNESSIONE ALLE SORGENTI MIDI
MIDIClient.init;
MIDIIn.connectAll;
~noteSet=64;
//RICERCA DEL PLUGIN JUCE
VSTPlugin.search;
//BUS AUDIO IN USCITA
(~myBus = Bus.audio(s, 2);
//Template della istanza del plugin

SynthDef(\effetto, { arg bus;
    //ReplaceOut.ar(bus, VSTPlugin.ar(In.ar(bus, 2), 2));
	ReplaceOut.ar(bus, VSTPlugin.ar(In.ar(bus, 2), numOut: `[2]));
}).add;
//TEMPLATE SEI 4 SYNTH
SynthDef("Sin", {
	arg freq = 440, pan = 0, mul=1, det =0.2, bus, amp=1, gate1=1, gate2=1;
	var out, detune;
	detune = SinOsc.kr(freq: 8.2.rand, mul:5*det);
	out = amp*EnvGen.kr(Env.adsr, gate1*gate2, doneAction: 0)*SinOsc.ar((freq - detune), mul: mul);
	Out.ar(bus, Pan2.ar(out,pan));
}).add;
SynthDef("Impulse",
 {arg freq=466, pan =0, width=0.1, mul=1, det=0.2, bus, amp=1, gate1=1, gate2=1;
	var sig, out, detune;
	detune = SinOsc.kr  (freq: 8.2.rand, mul:5*det);
	out = amp*EnvGen.kr(Env.adsr, gate1*gate2, doneAction: 0)*Pulse.ar (freq,width ,mul);
	Out.ar(bus, Pan2.ar(out,pan));
} ).add;//we add it to the server
SynthDef("Sawtooth", {
	arg freq = 493, pan = 0, mul=1, det= 0.2, bus, amp=1, gate1=1, gate2=1;
	var out, detune;
	detune = SinOsc.kr  (freq: 8.2.rand, mul:5*det);
	out = amp*EnvGen.kr(Env.adsr, gate1*gate2, doneAction: 0)*Saw.ar(freq, mul: mul);
	Out.ar(bus, Pan2.ar(out,pan));
}).add;
SynthDef("Square",
 {arg freq=466, pan =0, width=0.5, mul=1, det=0.2, bus, amp=1, gate1=1, gate2=1;
	var out, detune;
		detune = SinOsc.kr  (freq: 8.2.rand, mul:5*det);
	out = amp*EnvGen.kr(Env.adsr, gate1*gate2, doneAction: 0)*Pulse.ar (freq,width ,mul);
	Out.ar(bus, Pan2.ar(out,pan));
} ).add;//we add it to the server

//SELEZIONA LA FORMA D'ONDA DA UNA PORTA OSC
~r=NetAddr.new("127.0.0.1", 57120);
OSCdef(\synthSelection, {arg selector;
	postln("carciofo");
	selector[1].postln;
	switch(selector[1],
        1, {
            ~b1.free;
            ~b2.free;
            ~b3.free;
            ~b4.free;
            ~b5.free;
			~b1 = Synth.new(\Sin, [freq: ~noteSet, bus: ~myBus]);
			~b2 = Synth.new(\Sin, [freq: ~noteSet, bus: ~myBus]);
			~b3 = Synth.new(\Sin, [freq: ~noteSet, bus: ~myBus]);
			~b4 = Synth.new(\Sin, [freq: ~noteSet, bus: ~myBus]);
			~b5 = Synth.new(\Sin, [freq: ~noteSet, bus: ~myBus]);
        },
        2, {
			~b1.free;
            ~b2.free;
            ~b3.free;
            ~b4.free;
            ~b5.free;
			~b1 = Synth.new(\Impulse, [freq: ~noteSet, bus: ~myBus]);
			~b2 = Synth.new(\Impulse, [freq: ~noteSet, bus: ~myBus]);
			~b3 = Synth.new(\Impulse, [freq: ~noteSet, bus: ~myBus]);
			~b4 = Synth.new(\Impulse, [freq: ~noteSet, bus: ~myBus]);
			~b5 = Synth.new(\Impulse, [freq: ~noteSet, bus: ~myBus]);
        },
        3, {
            ~b1.free;
            ~b2.free;
            ~b3.free;
            ~b4.free;
            ~b5.free;
			~b1 = Synth.new(\Sawtooth, [freq: ~noteSet, bus: ~myBus]);
			~b2 = Synth.new(\Sawtooth, [freq: ~noteSet, bus: ~myBus]);
			~b3 = Synth.new(\Sawtooth, [freq: ~noteSet, bus: ~myBus]);
			~b4 = Synth.new(\Sawtooth, [freq: ~noteSet, bus: ~myBus]);
			~b5 = Synth.new(\Sawtooth, [freq: ~noteSet, bus: ~myBus]);
        },
        4, {
            ~b1.free;
            ~b2.free;
            ~b3.free;
            ~b4.free;
            ~b5.free;
			~b1 = Synth.new(\Square, [freq: ~noteSet, bus: ~myBus]);
			~b2 = Synth.new(\Square, [freq: ~noteSet, bus: ~myBus]);
			~b3 = Synth.new(\Square, [freq: ~noteSet, bus: ~myBus]);
			~b4 = Synth.new(\Square, [freq: ~noteSet, bus: ~myBus]);
			~b5 = Synth.new(\Square, [freq: ~noteSet, bus: ~myBus]);
        }
    );
}, '/dropdown');
OSCdef(\exit, {
	0.exit;
}, '/offMessage');
//PER TESTARE
//~b.set(\freq, 200);
//~b.set(\gate, 1);
//~fx.set(8, 0);
//INVIA MESSAGGIO OSC CON NOTA A PROCESSING
~n=NetAddr.new("127.0.0.1", 8000);
//RICEZIONE DELLE NOTE MIDI DA ARDUINO
MIDIdef.noteOn(\nota, {
	arg vel, note;
	~noteSet=note;
	~b1.set(\freq, ~noteSet.midicps);
	~b2.set(\freq, ~noteSet.midicps);
	~b3.set(\freq, ~noteSet.midicps);
	~b4.set(\freq, ~noteSet.midicps);
	~b5.set(\freq, ~noteSet.midicps);
	~b1.set(\gate1, 1);
	~b2.set(\gate1, 1);
	~b3.set(\gate1, 1);
	~b4.set(\gate1, 1);
	~b5.set(\gate1, 1);
	~n.sendMsg('/MidiNote', ~noteSet);
	postln(note);
});
//ATTIVAZIONE DELLA NOTA CON IL GATE
MIDIdef.noteOff(\noteOn, {
	~b1.set(\gate1, 0);
	~b2.set(\gate1, 0);
	~b3.set(\gate1, 0);
	~b4.set(\gate1, 0);
	~b5.set(\gate1, 0);
});
~n.sendMsg('/MidiNote', 35);

//SEZIONE PER RICEVERE E INVIARE MESSAGGI OSC A PROCESSING TRAMITE IL PROTOCOLLO OSC, HO UNA PORTA D'INGRESSO E UNA DI USCITA E L'INDIRIZZO IP PER INVIARLI ALLO STESSO HARDWARE è 127.0.0.1

OSCdef.all;
OSCdef.new(\detune, {arg detOsc;
	~b1.set(\det, detOsc);
	~b2.set(\det, detOsc);
	~b3.set(\det, detOsc);
	~b4.set(\det, detOsc);
	~b5.set(\det, detOsc);
	postln(detOsc);
}, '/Detune');
OSCdef.new(\unison, {arg g1, g2, g3, g4, g5, p1, p2, p3, p4, p5;
	~b1.set(\gate2, g1, \pan, p1);
	~b2.set(\gate2, g2, \pan, p2);
	~b3.set(\gate2, g3, \pan, p3);
	~b4.set(\gate2, g4, \pan, p4);
	~b5.set(\gate2, g5, \pan, p5);
	postln(g1);
}, '/Unison');
OSCdef.new(\vol, {arg volOsc;
	~b1.set(\amp, volOsc);
	~b2.set(\amp, volOsc);
	~b3.set(\amp, volOsc);
	~b4.set(\amp, volOsc);
	~b5.set(\amp, volOsc);
	postln(volOsc);
}, '/Volume');
)


//APRO IL PLUGIN, LO ASSOCIO A UN UGEN E LO CONTROLLO CON VSTCONTROLLER
~fx = VSTPluginController(Synth(\effetto, [\bus, ~myBus])).open("RVerb Mono/Stereo");
/*~fx.set(8, 1);
~b = Synth.new("Sin", [freq:~noteSet,bus:~myBus]);//(-1 + (a * i)), ~myBus).play;
~b.free;//ricordati di deallocare
~b.set(\freq, 200);*/
//APERTURA GUI DEL PLUGIN(NON NECESSARIA)
~fx.gui;
~fx.editor;
//RICEZIONE DELLE FREQUENZE DEI FILTRI DA ARDUINO
MIDIdef.cc(\filter1, {
	arg val;
	~fx.set(0, val);   //0 per la frequenza del primo filtro
}, 12);
MIDIdef.cc(\filter2, {
	arg val;
	~fx.set(1, val);   //1 per la frequenza del primo filtro
}, 13);
//SUONA IL BUS
~myBus.play;