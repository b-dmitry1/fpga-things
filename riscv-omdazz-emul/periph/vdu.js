Vdu = function() {
	this.width = 640;
	this.height = 480;
	this.bpp = 4;
	this.colorMask = (1 << this.bpp) - 1;
	this.frameSize = this.width * this.height * this.bpp / 8;
	this.front = 0;

	this.red   = [0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF];
	this.green = [0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF];
	this.blue  = [0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF];

	this.drawPixels = function(data, ofs, value) {
		for (let i = 0; i < 32 / this.bpp; i++, ofs += 4) {
			let color = value & this.colorMask;
			value >>= this.bpp;
			data[ofs + 0] = this.red  [color];
			data[ofs + 1] = this.green[color];
			data[ofs + 2] = this.blue [color];
			data[ofs + 3] = 0xFF;
		}
	}

	this.update = function() {
		const ramdev = sys.getDevice(0x80);

		if ("function" != typeof ramdev.getRam)
			return;

		const ram = ramdev.getRam();

		let front = uint(this.front);

		if (front + this.frameSize > ram.length * 4)
			return;

		front /= 4;

		const canvas = document.getElementById('display');
		const ctx = canvas.getContext("2d");

		const pixels = ctx.getImageData(0, 0, this.width, this.height);
		const data = pixels.data;

		for (let i = 0, ofs = 0; i < this.frameSize / 4; i++, ofs += 32 / this.bpp) {
			this.drawPixels(data, ofs * 4, ram[front + i]);
		}

		ctx.putImageData(pixels, 0, 0);
	};

	this.read8 = function(addr) {
		return 1;
	};

	this.write8 = function(addr, value) {
	};

	this.read32 = function(addr) {
		return this.front | 0x01;
	};

	this.write32 = function(addr, value) {
		if ((addr & 0xFFFFFF) == 0) {
			this.front = uint(value & 0x0FFFFFF0);
		}
		this.update();
	};
};
