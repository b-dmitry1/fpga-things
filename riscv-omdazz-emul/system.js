RiscvSystem = function() {
	//////////////////////////////////////////////////////////////////////////////
	// PUBLIC
	// mapDevice
	//////////////////////////////////////////////////////////////////////////////
	this.mapDevice = function(highAddrByte, device) {
		this.devices[highAddrByte & 0xFF] = device;
	};


	//////////////////////////////////////////////////////////////////////////////
	// PRIVATE
	// do not use directly
	//////////////////////////////////////////////////////////////////////////////

	this.devices = [];

	for (let i = 0; i < 256; i++)
		this.devices[i] = 0;

	this.read8 = function(addr) {
		addr &= 0xFFFFFFFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0)
			return dev.read8(addr);
		return 0;
	};

	this.read32 = function(addr) {
		addr &= 0xFFFFFFFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0)
			return dev.read32(addr);
		return 0;
	};

	this.write8 = function(addr, value) {
		addr &= 0xFFFFFFFF;
		value &= 0xFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0)
			dev.write8(addr, value);
	};

	this.write16 = function(addr, value) {
		addr &= 0xFFFFFFFF;
		value &= 0xFFFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0) {
			dev.write8(addr, value & 0xFF);
			dev.write8(addr + 1, (value >> 8) & 0xFF);
		}
	};

	this.write32 = function(addr, value) {
		addr &= 0xFFFFFFFF;
		value &= 0xFFFFFFFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0)
			dev.write32(addr, value);
	};
};
