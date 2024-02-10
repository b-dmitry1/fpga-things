RiscvSystem = function() {
	//////////////////////////////////////////////////////////////////////////////
	// PUBLIC
	// mapDevice, getDevice
	//////////////////////////////////////////////////////////////////////////////
	this.mapDevice = function(highAddrByte, device) {
		this.devices[highAddrByte & 0xFF] = device;
	};

	this.getDevice = function(highAddrByte) {
		return this.devices[highAddrByte & 0xFF];
	}

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

	this.read16 = function(addr) {
		addr &= 0xFFFFFFFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0)
			return dev.read8(addr) | (dev.read8(addr + 1) << 8);
		return 0;
	};

	this.read32 = function(addr) {
		addr &= 0xFFFFFFFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0) {
			if ((addr & 3) == 0)
				return dev.read32(addr);
			return dev.read8(addr) |
				(dev.read8(addr + 1) << 8) |
				(dev.read8(addr + 2) << 16) |
				(dev.read8(addr + 3) << 24);
		}
		return 0;
	};

	this.write8 = function(addr, value) {
		addr &= 0xFFFFFFFF;
		value = uint(value);
		value &= 0xFF;
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0)
			dev.write8(addr, value);
	};

	this.write16 = function(addr, value) {
		addr &= 0xFFFFFFFF;
		value = uint(value);
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
		value = uint(value);
		let index = (addr >> 24) & 0xFF;
		let dev = this.devices[index];
		if (dev != 0) {
			if ((addr & 0x03) == 0) {
				dev.write32(addr, value);
			} else {
				dev.write8(addr, value & 0xFF);
				dev.write8(addr + 1, (value >> 8) & 0xFF);
				dev.write8(addr + 2, (value >> 16) & 0xFF);
				dev.write8(addr + 3, (value >> 24) & 0xFF);
			}
		}
	};
};
