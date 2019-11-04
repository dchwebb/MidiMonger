if (navigator.requestMIDIAccess) {
	console.log('This browser supports WebMIDI');
} else {
	console.log('WebMIDI is not supported in this browser.');
}

var midi = null;			// global MIDIAccess object
var output = null;
var requestNo = 1;			// stores number of control awaiting configuration information

var cfgEnum = { type: 1, specificNote: 2, channel: 3, controller: 4 };

window.onload = afterLoad;
function afterLoad() {

	for (var g = 1; g < 9; g++) {

		// Generate html for gate configuration controls
		var html = [
			'<div style = "padding: 5px;">Type</div>', 
			'<div><select id="gType' + g + '" class="docNav" onchange="updateGate(' + g + ', cfgEnum.type);"></select></div>',
			'<div class="gateControl" style = "padding: 5px;">Note</div>', 
			'<div class="gateControl"><select id="gNote' + g + '" class="docNav" onchange="updateGate(' + g + ', cfgEnum.specificNote);"></select></div>', 
			'<div class="channelControl" style = "padding: 5px;">Channel</div>', 
			'<div class="channelControl"><select id="gChannel' + g + '" class="docNav" onchange="updateGate(' + g + ', cfgEnum.channel);"></select></div>', 
			'<div class="grid-span">', 
			'	<button class="topcoat-button-bar__button--large" onclick="sendNote(gNote' + g + '.value, gChannel' + g + '.value);">Test</button>', 
			'</div>' 
		].join("\n");
		document.getElementById("gridCtl" + g).innerHTML = html;

		// populate type pickers
		var typeNames = ["Specific Note", "Channel", "Clock"]
		for(var i = 0; i < 3; i++) {
			var el = document.createElement("option");
			el.textContent = typeNames[i];
			el.value = i + 1;
			document.getElementById("gType" + g).appendChild(el);
		}


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

// Recieve MIDI message - mainly used to process configuration information returned from module
function getMIDIMessage(midiMessage) {
	console.log(midiMessage);

	if (midiMessage.data[0] == 0xF2) {
		var type = (midiMessage.data[1] & 0xF0) >> 4;
		document.getElementById("gType" + requestNo).value = type;
		document.getElementById("gChannel" + requestNo).value = (midiMessage.data[1] & 0xF) - 1;
		document.getElementById("gNote" + requestNo).value = midiMessage.data[2];
		updateDisplay(requestNo);

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
function updateGate(gateNo, cfgType) {
	updateDisplay(gateNo);

	switch (cfgType) {
	case cfgEnum.type :
		var gType = document.getElementById("gType" + gateNo);
		var cfgValue = gType.options[gType.selectedIndex].value;
		break;
	case cfgEnum.specificNote :
		var gNote = document.getElementById("gNote" + gateNo);
		var cfgValue = gNote.options[gNote.selectedIndex].value;
		break;
	case cfgEnum.channel :
		var gChannel = document.getElementById("gChannel" + gateNo);
		var cfgValue = gChannel.options[gChannel.selectedIndex].value;
		break;
	case cfgEnum.controller :
		var gController = document.getElementById("gController" + gateNo);
		var cfgValue = gController.options[gController.selectedIndex].value;
		break;
	}
	// to send config message use format: ggggtttt vvvvvvvv where g is the gate number and t is the type of setting to pass
	var message = [0xF2, (cfgType << 4) + gateNo, parseInt(cfgValue)];		// Use MIDI song position to send patch data
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


var gateEnum = { specificNote: 1, channelNote: 2, clock: 3 };

// Updates configuration blocks to show/hide irrelevant settings
function updateDisplay(gateNo) {
	// specific note picker
	var gateShow =  document.getElementById("gType" + gateNo).value == gateEnum.specificNote ? "block" : "none";
	var gateControls = document.getElementById("gridCtl" + gateNo).getElementsByClassName("gateControl");
	for (var i = 0; i < gateControls.length; i++) {
		gateControls[i].style.display = gateShow;
	}

	// channel picker
	var channelShow =  document.getElementById("gType" + gateNo).value != gateEnum.clock ? "block" : "none";
	var channelControls = document.getElementById("gridCtl" + gateNo).getElementsByClassName("channelControl");
	for (var i = 0; i < gateControls.length; i++) {
		channelControls[i].style.display = channelShow;
	}
}