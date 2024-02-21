textcon = function() {
	this.width = 80;
	this.height = 25;
	this.x = 0;
	this.y = 0;
	this.text = [];
	this.escape = [];

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

	this.doBackspace = function(con) {
		let line = con.text[con.y];
		if (con.x > 0)
			con.x--;
		if (line.length > 0)
			con.text[con.y] = line.substring(0, con.x) + line.substring(con.x + 1);
		con.updateLine(con.y);
	};

	this.doEscape = function(last) {
		switch (this.escape) {
			case '\x1b[J':
				this.doBackspace(this);
				this.escape = '';
				return;
		}

		if (last == 'm') {
			this.escape = '';
		}

		if (this.escape.length > 10)
			this.escape = '';
	};

	this.putchar = function(ch) {
		if (ch == '\b') {
			this.escape = '';
			return;
		}
		if (ch == '\x1b') {
			this.doEscape(ch);
			this.escape = '\x1b';
			return;
		}
		if (this.escape.length > 0) {
			this.escape += ch;
			this.doEscape(ch);
			return;
		}
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
				line = line.substring(0, this.x) + ch + line.substring(this.x + 1);
			}
			this.text[this.y] = line;
			this.x++;
			this.updateLine(this.y);
		}
	};
};
