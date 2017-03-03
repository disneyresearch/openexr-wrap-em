var fs = require("fs");
var exr = require("./exr-wrap.js");

var data = fs.readFileSync(process.argv[2]);

for (var i = 0; i < 10; ++i)
{
	var image = exr.loadEXRStr(data);

	console.log(image.width, image.height);
	console.log(image.channels());
	//console.log(image.plane("R"));

	image.delete()
}
