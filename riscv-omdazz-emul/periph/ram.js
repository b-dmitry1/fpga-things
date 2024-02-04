Ram = function(size) {
	// RAM
	this.size = size & 0xFFFFFFFF;
	this.ram = new Uint32Array(size >> 2);
	this.addrMask = (size - 1) & 0xFFFFFFFF;

	this.read8 = function(addr) {
		let rv = this.ram[(addr & this.addrMask) >> 2];
		switch (addr & 0x03) {
			case 0: rv >>= 0; break;
			case 1: rv >>= 8; break;
			case 2: rv >>= 16; break;
			case 3: rv >>= 24; break;
		}
		return rv & 0xFF;
	};

	this.write8 = function(addr, value) {
		let rv = this.ram[(addr & this.addrMask) >> 2];
		switch (addr & 0x03) {
			case 0: rv = (rv & 0xFFFFFF00) | ((value << 0) & 0xFF); break;
			case 1: rv = (rv & 0xFFFF00FF) | ((value << 8) & 0xFF00); break;
			case 2: rv = (rv & 0xFF00FFFF) | ((value << 16) & 0xFF0000); break;
			case 3: rv = (rv & 0x00FFFFFF) | ((value << 24) & 0xFF000000); break;
		}
		this.ram[(addr & this.addrMask) >> 2] = rv;
	};

	this.read32 = function(addr) {
		addr &= 0xFFFFFFFF;
		return this.ram[(addr & this.addrMask) >> 2];
	};

	this.write32 = function(addr, value) {
		addr &= 0xFFFFFFFF;
		this.ram[(addr & this.addrMask) >> 2] = value & 0xFFFFFFFF;
	};
};
