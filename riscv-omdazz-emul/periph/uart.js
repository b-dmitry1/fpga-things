Uart = function() {
	this.read8 = function(addr) {
		switch (addr & 7) {
			case 5:
				return 0x60;
		}
		return 0;
	};

	this.write8 = function(addr, value) {
		switch (addr & 7) {
			case 0:
				putchar(String.fromCharCode(value & 0xFF));
				break;
		}
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
};
