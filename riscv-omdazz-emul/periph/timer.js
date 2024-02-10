Timer = function() {
	this.mtime = 0;
	this.mtimecmp = 0;
	this.irq = false;

	this.tick = function(us) {
		this.mtime += us;
		this.irq = (this.mtimecmp > 0) && (this.mtime > this.mtimecmp);
	};

	this.read8 = function(addr) {
		return 0;
	};

	this.write8 = function(addr, value) {
	};

	this.read32 = function(addr) {
		switch (addr & 0xFFFF) {
			case 0x4000:
				return this.mtimecmp & 0xFFFFFFFF;
			case 0x4004:
				temp = this.mtimecmp;
				temp /= 65536;
				temp /= 65536;
				return temp;
			case 0xBFF8:
				return this.mtime & 0xFFFFFFFF;
			case 0xBFFC:
				let temp = this.mtime;
				temp /= 65536;
				temp /= 65536;
				return temp;
		}
		return 0;
	};

	this.write32 = function(addr, value) {
		switch (addr & 0xFFFF) {
			case 0x4000:
				temp = this.mtimecmp;
				temp /= 65536;
				temp /= 65536;
				temp *= 65536;
				temp *= 65536;
				temp |= value & 0xFFFFFFFF;
				this.mtimecmp = temp;
				break;
			case 0x4004:
				temp = this.mtimecmp;
				temp *= 65536;
				temp *= 65536;
				temp |= value & 0xFFFFFFFF;
				//this.mtimecmp = temp;
				break;
			case 0xBFF8:
				temp = this.mtime;
				temp /= 65536;
				temp /= 65536;
				temp *= 65536;
				temp *= 65536;
				temp |= value & 0xFFFFFFFF;
				this.mtime = temp;
				break;
			case 0xBFFC:
				temp = this.mtime;
				temp *= 65536;
				temp *= 65536;
				temp |= value & 0xFFFFFFFF;
				//this.mtime = temp;
				break;
		}
	};
};
