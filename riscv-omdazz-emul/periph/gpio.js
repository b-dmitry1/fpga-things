Gpio = function() {
	this.inputs = 0;
	this.outputs = 0;

	this.read8 = function(addr) {
		return this.inputs;
	};

	this.write8 = function(addr, value) {
		this.outputs = value & 0xFF;
	};

	this.read32 = function(addr) {
		return this.inputs;
	};

	this.write32 = function(addr, value) {
		this.outputs = value;
	};
};
