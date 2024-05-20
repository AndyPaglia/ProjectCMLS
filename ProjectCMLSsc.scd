//setup
s.boot;
//CONNESSIONE ALLE SORGENTI MIDI
MIDIClient.init;
MIDIIn.connectAll;
~noteSet=1000;
//RICERCA DEL PLUGIN JUCE
VSTPlugin.search;
//BUS AUDIO IN USCITA
~myBus = Bus.audio(s, 2);
//Template della istanza del plugin
(
SynthDef(\effetto, { arg bus;
    //ReplaceOut.ar(bus, VSTPlugin.ar(In.ar(bus, 2), 2));
	ReplaceOut.ar(bus, VSTPlugin.ar(In.ar(bus, 2), numOut: `[2]));
}).add;
)
//TEMPLATE SEI 4 SYNTH
(
SynthDef("Sin", {
	arg freq = 440, pan = 0, mul=1, det =0.2, bus;
	var out, detune;
	detune = SinOsc.kr(freq: 8.2.rand, mul:5*det);
	out = SinOsc.ar((freq - detune), mul: mul);
	Out.ar(bus, Pan2.ar(out,pan));
}).add)
(
SynthDef("Impulse",
 {arg freq=466, pan =0, width=0.1, mul=1, det=0.2, bus, gate=0 ;
	var sig, out, detune;
	detune = SinOsc.kr  (freq: 8.2.rand, mul:5*det);
	out = EnvGen.kr(Env.adsr, gate, doneAction: 0)*Pulse.ar (freq,width ,mul);
	Out.ar(bus, Pan2.ar(out,pan));
} ).add;//we add it to the server
)
(
SynthDef("Sawtooth", {
	arg freq = 493, pan = 0, mul=1, det= 0.2, bus, gate=0;
	var out, detune;
	detune = SinOsc.kr  (freq: 8.2.rand, mul:5*det);
	out = EnvGen.kr(Env.adsr, gate, doneAction: 0)*Saw.ar(freq, mul: mul);
	Out.ar(bus, Pan2.ar(out,pan));
}).add)
(SynthDef("Square",
 {arg freq=466, pan =0, width=0.5, mul=1, det=0.2, bus, gate=0;
	var out, detune;
		detune = SinOsc.kr  (freq: 8.2.rand, mul:5*det);
	out = EnvGen.kr(Env.adsr, gate, doneAction: 0)*Pulse.ar (freq,width ,mul);
	Out.ar(bus, Pan2.ar(out,pan));
} ).add;//we add it to the server
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
//SUONA IL BUS
~myBus.play;

//SELEZIONA LA FORMA D'ONDA DA UNA PORTA OSC
~r=NetAddr.new("127.0.0.1", 57120);
OSCdef(\synthSelection, {arg selector;
	postln("carciofo");
	selector[1].postln;
	switch(selector[1],
        1, {
            b.free;
			b = Synth.new(\Sin, [freq: ~noteSet, bus: ~myBus]);
        },
        2, {
            b.free;
			b = Synth.new(\Impulse, [freq: ~noteSet, bus: ~myBus]);
        },
        3, {
            b.free;
			b = Synth.new(\Sawtooth, [freq: ~noteSet, bus: ~myBus]);
        },
        4, {
            b.free;
			b = Synth.new(\Square, [freq: ~noteSet, bus: ~myBus]);
        }
    );
}, '/menu');


//RICEZIONE DELLE NOTE MIDI DA ARDUINO
MIDIdef.noteOn(\nota, {
	arg vel, note;
	~noteSet=note;
	~b.set(\freq, ~noteSet);
});
//INVIA MESSAGGIO OSC CON NOTA A PROCESSING
~n=NetAddr.new("127.0.0.1", 8000);
~n.sendMsg(
	'/frequenzaNota', ~noteSet
);
//RICEZIONE DELLE FREQUENZE DEI FILTRI DA ARDUINO
MIDIdef.cc(\filter1, {
	arg val;
	~fx.set(0, val);   //0 per la frequenza del primo filtro
}, 12);
MIDIdef.cc(\filter2, {
	arg val;
	~fx.set(1, val);   //1 per la frequenza del primo filtro
}, 13);
//ATTIVAZIONE DELLA NOTA CON IL GATE
MIDIdef.cc(\noteOn, {
	arg On;
	if (On==1, {b.set(\gate, 1)
}, {b.set(\gate, 0)});,
},	noteonChannel);


//SEZIONE PER RICEVERE E INVIARE MESSAGGI OSC A PROCESSING TRAMITE IL PROTOCOLLO OSC, HO UNA PORTA D'INGRESSO E UNA DI USCITA E L'INDIRIZZO IP PER INVIARLI ALLO STESSO HARDWARE è 127.0.0.1

OSCdef.all;
OSCdef(\detune, {arg detOsc;
	~b.set(\det, detOsc);
}, '/ang');
OSCdef(\unison, {arg uniOsc;
	~unison=uniOsc;
}, '/ang');
OSCdef(\vol, {arg volOsc;
	~amplitude=volOsc;
}, '/volume');