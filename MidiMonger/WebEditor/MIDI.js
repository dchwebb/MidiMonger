if (navigator.requestMIDIAccess) {
	console.log('This browser supports WebMIDI!');
} else {
	console.log('WebMIDI is not supported in this browser.');
}

var midi = null;			// global MIDIAccess object
var output = null;

// sysex not currently working so no need request permission: requestMIDIAccess({ sysex: true })
navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);

function onMIDISuccess(midiAccess) {
	midi = midiAccess;		// store in the global variable

	console.log(midiAccess);

	var inputs = midiAccess.inputs;
	var outputs = midiAccess.outputs;


	for (var input of midiAccess.inputs.values()) {
		console.log(input.name);
		if (input.name == "Mountjoy MIDI") {
			input.onmidimessage = getMIDIMessage;
		}
	}
	for (var out of midiAccess.outputs.values()) {
		if (out.name == "Mountjoy MIDI") {
			output = out;
		}
	}
}

function onMIDIFailure() {
	console.log('Could not access your MIDI devices.');
}

function getMIDIMessage(midiMessage) {
	console.log(midiMessage);
}

function sendMiddleC() {
	var noteOnMessage = [0x90, 60, 0x7f];		// note on, middle C, full velocity
	output.send(noteOnMessage);					//omitting the timestamp means send immediately.
	
	// Inlined array creation- note off, middle C, release velocity = 64, timestamp = now + 1000ms.
	output.send([0x80, 60, 0x40], window.performance.now() + 1000.0);
}

function sendSysEx() {
	var message = [0xF2, 0x2A, 0x2D];		// Use MIDI song position to send patch data
	output.send(message);
}
