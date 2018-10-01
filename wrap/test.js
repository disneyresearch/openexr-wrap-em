var fs = require("fs");
var exr = require("./exr-wrap.js");

exr().then(function(loader) {
	var data = fs.readFileSync(process.argv[2]);
	for (var i = 0; i < 100; ++i) {
		var image = loader.loadEXRStr(data);

		console.log(image.width, image.height);
		console.log(image.channels());
		let r = image.plane("R");
		let g = image.plane("G");
		let b = image.plane("B");

		image.delete();
	}
});
