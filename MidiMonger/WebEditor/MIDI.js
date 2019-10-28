if (navigator.requestMIDIAccess) {
	console.log('This browser supports WebMIDI!');
} else {
	console.log('WebMIDI is not supported in this browser.');
}

var midi = null;			// global MIDIAccess object
var output = null;

// sysex not currently working so no need request permission: requestMIDIAccess({ sysex: true })
navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);

window.onload = afterLoad;
function afterLoad() {

	// populate note pickers
	var noteNames = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
	var select = document.getElementById("gNote1");
	for(var i = 24; i < 97; i++) {
		var el = document.createElement("option");
		el.textContent = noteNames[i % 12] + (Math.floor(i / 12) - 1);	// C1 (24) to C7 (96)
		el.value = i;
		select.appendChild(el);
	}

	// populate channel pickers
	var select = document.getElementById("gNote1");
	for(var i = 0; i < 16; i++) {
		var el = document.createElement("option");
		el.textContent = i + 1;
		el.value = i;
		document.getElementById("gChannel1").appendChild(el);
	}

}


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
			var status = document.getElementById("connectionStatus");
			status.innerHTML = "Connected"
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

function sendNote(noteValue, channel) {
	var noteOnMessage = [0x90 + parseInt(channel), noteValue, 0x7f];		// note on, middle C, full velocity
	output.send(noteOnMessage);					//omitting the timestamp means send immediately.
	
	// Inlined array creation- note off, middle C, release velocity = 64, timestamp = now + 1000ms.
	output.send([0x80, noteValue, 0x40], window.performance.now() + 1000.0);
}


function sendSysEx() {
	var message = [0xF2, 0x2A, 0x2D];		// Use MIDI song position to send patch data
	output.send(message);
}

function updateGate(gateNo) {
	var gNote = document.getElementById("gNote" + gateNo);
	var noteVal = gNote.options[gNote.selectedIndex].value;

	var gChannel = document.getElementById("gChannel" + gateNo);
	var channelVal = gChannel.options[gChannel.selectedIndex].value;


	var message = [0xF2, noteVal, channelVal];		// Use MIDI song position to send patch data
	output.send(message);
}
