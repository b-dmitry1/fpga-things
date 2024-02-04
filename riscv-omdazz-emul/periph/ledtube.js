LedTube = function() {
	this.digits = [];

	for (i = 0; i < 4; i++)
		this.digits[i] = 0;

	this.update = function(digit) {
		let canvas = document.getElementById('ledtube');
		const ctx = canvas.getContext("2d");

		let x = 10 + (digit ^ 3) * 80;
		let y = 10;
		let w = 32;
		let sw = 5;
		let sh = 40;
		ctx.fillStyle = this.digits[digit] & 0x01 ? "#F77" : '#444';
		ctx.fillRect(x + sw, y, sh, sw);
		ctx.fillStyle = this.digits[digit] & 0x02 ? "#F77" : '#444';
		ctx.fillRect(x + sw + sh, y + sw, sw, sh);
		ctx.fillStyle = this.digits[digit] & 0x04 ? "#F77" : '#444';
		ctx.fillRect(x + sw + sh, y + sw + sh + sw, sw, sh);
		ctx.fillStyle = this.digits[digit] & 0x08 ? "#F77" : '#444';
		ctx.fillRect(x + sw, y + sw + sh + sw + sh, sh, sw);
		ctx.fillStyle = this.digits[digit] & 0x10 ? "#F77" : '#444';
		ctx.fillRect(x, y + sw + sh + sw, sw, sh);
		ctx.fillStyle = this.digits[digit] & 0x20 ? "#F77" : '#444';
		ctx.fillRect(x, y + sw, sw, sh);
		ctx.fillStyle = this.digits[digit] & 0x40 ? "#F77" : '#444';
		ctx.fillRect(x + sw, y + sw + sh, sh, sw);
		ctx.fillStyle = this.digits[digit] & 0x80 ? "#F77" : '#444';
		ctx.fillRect(x + sw + sh + sw + sw, y + sw + sh + sw + sh, sw, sw);
	};

	this.read8 = function(addr) {
		return this.digits[addr & 3];
	};

	this.write8 = function(addr, value) {
		this.digits[addr & 3] = value & 0xFF;
		this.update(addr & 3);
	};

	this.read32 = function(addr) {
		let res = 0;
		res |= this.read8(addr) << 0;
		res |= this.read8(addr + 1) << 8;
		res |= this.read8(addr + 2) << 16;
		res |= this.read8(addr + 3) << 24;
		return res;
	};

	this.write32 = function(addr, value) {
		this.write8(addr, value & 0xFF);
		this.write8(addr + 1, (value >> 8) & 0xFF);
		this.write8(addr + 2, (value >> 16) & 0xFF);
		this.write8(addr + 3, (value >> 24)  & 0xFF);
	};

	for (i = 0; i < 4; i++)
		this.update(i);

	let canvas = document.getElementById('ledtube');

	const ctx = canvas.getContext("2d");
	ctx.fillStyle = "#333";
	ctx.fillRect(0, 0, canvas.width, canvas.height);
};
