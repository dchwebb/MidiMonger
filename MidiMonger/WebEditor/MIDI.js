if (navigator.requestMIDIAccess) {
	console.log('This browser supports WebMIDI');
} else {
	console.log('WebMIDI is not supported in this browser.');
}

var midi = null;			// global MIDIAccess object
var output = null;
var requestNo = 1;			// stores number of control awaiting configuration information

window.onload = afterLoad;
function afterLoad() {

	for (var g = 1; g < 9; g++) {

		// Generate html for gate configuration controls
		var html = [
			'<div style = "padding: 5px;">Note</div>', 
			'<div><select id="gNote' + g + '" class="docNav" onchange="updateGate(' + g + ');"></select></div>', 
			'<div style = "padding: 5px;">Channel</div>', 
			'<div><select id="gChannel' + g + '" class="docNav" onchange="updateGate(' + g + ');"></select></div>', 
			'<div class="grid-span">', 
			'	<button class="topcoat-button-bar__button--large" onclick="sendNote(gNote' + g + '.value, gChannel' + g + '.value);">Test</button>', 
			'</div>' 
		].join("\n");
		document.getElementById("gridCtl" + g).innerHTML = html;

		// populate note pickers
		var noteNames = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
		for(var i = 24; i < 97; i++) {
			var el = document.createElement("option");
			el.textContent = noteNames[i % 12] + (Math.floor(i / 12) - 1);	// C1 (24) to C7 (96)
			el.value = i;
			document.getElementById("gNote" + g).appendChild(el);
		}

		// populate channel pickers
		for(var i = 0; i < 16; i++) {
			var el = document.createElement("option");
			el.textContent = i + 1;
			el.value = i;
			document.getElementById("gChannel" + g).appendChild(el);
		}
	}

	// sysex not currently working so no need request permission: requestMIDIAccess({ sysex: true })
	navigator.requestMIDIAccess().then(onMIDISuccess, onMIDIFailure);
}


// Check which MIDI interface is MIDI Monger and if found start requesting configuration from interface
function onMIDISuccess(midiAccess) {
	midi = midiAccess;		// store in the global variable

	console.log(midiAccess);

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

	// Update UI to show connection status
	if (checkConnection()) {
		getGate(1);			// get configuration for first port
	}
}

function onMIDIFailure() {
	console.log('Could not access your MIDI devices.');
}

function getMIDIMessage(midiMessage) {
	console.log(midiMessage);

	if (midiMessage.data[0] == 0xF2) {
		var type = (midiMessage.data[1] & 0xF0) >> 4;
		document.getElementById("gChannel" + requestNo).value = (midiMessage.data[1] & 0xF) - 1;
		document.getElementById("gNote" + requestNo).value = midiMessage.data[2];

		if (requestNo < 8) {
			requestNo++;
			getGate(requestNo);
		}
	}
}


// Sends MIDI note to requested channel
function sendNote(noteValue, channel) {
	if (checkConnection()) {
		output.send( [0x90 + parseInt(channel), noteValue, 0x7f]);
		output.send([0x80, noteValue, 0x40], window.performance.now() + 1000.0);	// note off delay: now + 1000ms
	}
}


// Request configuration for gate (uses MIDI song position mechanism)
function getGate(gateNo) {
	var message = [0xF2, parseInt(gateNo), 0x00];
	output.send(message);
}


// Send out a configuration change to a specific gate
function updateGate(gateNo) {
	var gNote = document.getElementById("gNote" + gateNo);
	var noteVal = gNote.options[gNote.selectedIndex].value;

	var gChannel = document.getElementById("gChannel" + gateNo);
	var channelVal = gChannel.options[gChannel.selectedIndex].value;

	var message = [0xF2, noteVal, channelVal];		// Use MIDI song position to send patch data
	output.send(message);
}


// Test the MIDI connection is active and update UI accordingly
function checkConnection() {
	var status = document.getElementById("connectionStatus");
	if (!output || output.state == "disconnected") {
		status.innerHTML = "Disconnected";
		status.style.color = "#d6756a";
		return false;
	} else {
		status.innerHTML = "Connected";
		status.style.color = "#7dce73";
		return true;
	}
}