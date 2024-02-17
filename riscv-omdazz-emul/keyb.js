function createKeyboard() {
	let shift = false;
	let caps = false;

	let keys = [
		['Esc', 'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12'],
		['`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '<-'],
		['Tab', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\'],
		['Caps', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'Enter'],
		['Shift', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 'Up', 'Shift'],
		['Ctrl', 'Alt', 'space', 'Alt', 'Ctrl', 'Left', 'Down', 'Right']
	];

	let keysShift = [
		['Esc', 'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12'],
		['~', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '<-'],
		['Tab', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '|'],
		['Caps', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', 'Enter'],
		['Shift', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 'Up', 'Shift'],
		['Ctrl', 'Alt', 'space', 'Alt', 'Ctrl', 'Left', 'Down', 'Right']
	];

	this.updateKeyboard = function() {
		let sh = this.caps;
		if (this.shift) sh = !sh;
		for (let i = 0; i < keys.length; i++) {
			for (let j = 0; j < keys[i].length; j++) {
				let button = document.getElementById('key_' + i + '_' + j);
				button.innerText = sh ? keysShift[i][j] : keys[i][j];
			}
		}
	};

	for (let i = 0; i < keys.length; i++) {
		let container = document.getElementById('keyb');

		let line = document.createElement('br');
		container.appendChild(line);

		for (let j = 0; j < keys[i].length; j++) {
			let button = document.createElement('button');
			button.id = 'key_' + i + '_' + j;
			button.innerText = keys[i][j];
			button.style.fontSize = '14pt';
			button.style.padding = '10px 20px';
			if (keys[i][j] == 'space')
				button.style.padding = '10px 100px';

			button.addEventListener('click', () => {
				let uart = sys.getDevice(0x10);
				if ("function" != typeof uart.addKey)
					return;

				let code = button.innerText;
				if (code == 'Caps') {
					this.caps = !this.caps;
					this.updateKeyboard();
					return;
				}
				if (code == 'Shift') {
					this.shift = !this.shift;
					this.updateKeyboard();
					return;
				}

				switch (code) {
					case 'Esc': code = 27; break;
					case 'Tab': code = '\t'; break;
					case 'Enter': code = '\r'; break;
					case '<-': code = '\b'; break;
					case 'space': code = ' '; break;
					case 'Up': code = '\x1b[A'; break;
					case 'Left': code = '\x1b[D'; break;
					case 'Down': code = '\x1b[B'; break;
					case 'Right': code = '\x1b[C'; break;
				}
				for (let c = 0; c < code.length; c++)
					uart.addKey(code[c]);
			});
			container.appendChild(button);
		}
	}
};
