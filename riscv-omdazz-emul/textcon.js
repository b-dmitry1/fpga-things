textcon = function() {
	this.width = 80;
	this.height = 25;
	this.x = 0;
	this.y = 0;
	this.text = [];

	// Create buffer and HTML lines
	let container = document.getElementById('textcon');
	let html = "";
	for (let i = 0; i < this.height; i++) {
		this.text[i] = ' '.repeat(this.width);
		html = html + "<div id='con" + i + "'>&nbsp;</div>";
	}
	container.innerHTML = html;
	container.style.width = '800px'; // TODO: Calc real width

	this.updateLine = function(index) {
		let line = document.getElementById('con' + index);
		line.innerHTML = '&nbsp;' + this.text[index];
	};

	this.update = function() {
		for (let i = 0; i < this.height; i++) {
			this.updateLine(i);
		}
	};

	this.shiftUp = function() {
		for (let i = 0; i < this.height - 1; i++) {
			this.text[i] = this.text[i + 1];
		}
		this.text[this.height - 1] = ' '.repeat(this.width - 1);
	};

	this.putchar = function(ch) {
		if (ch == '\r') {
			this.x = 0;
		} else if (ch == '\n') {
			this.x = 0;
			this.y++;
			if (this.y >= this.height) {
				this.shiftUp();
				this.y--;
			}
			this.update();
		} else {
			if (this.x >= this.width) {
				this.x = 0;
				this.y++;
				if (this.y >= this.height) {
					this.shiftUp();
					this.y--;
				}
				this.update();
			}
			let line = this.text[this.y];
			if (line.length <= this.x) {
				line += ch;
			} else if (this.x == 0) {
				line = ch + line.slice(1);
			} else {
				line = line.slice(0, this.x) + ch + line.slice(this.x);
			}
			this.text[this.y] = line;
			this.x++;
			this.updateLine(this.y);
		}
	};
};
