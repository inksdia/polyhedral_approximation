var express = require('express');
var multer  = require('multer');
const execFile = require('child_process').execFile;
var inputFile;

var upload = multer({ dest: './algo' });
var router = express.Router();

router.post('/uploads', upload.single('file'), function(req, res, next) {
	inputFile = req.file.filename;
	res.json(req.file);
	res.status(200).end();
});

router.post('/solve', function(req, res) {
	var reqbody = req.body;
	var args = [];
	args.push(inputFile);
	args.push(reqbody['distance']);
	args.push(reqbody['x']);
	args.push(reqbody['y']);
	args.push(reqbody['z']);
  args.push(reqbody['h']);
  args.push(reqbody['w']);
	args.push('../public/output.svg');
	console.log(args);
	const child = execFile('./a.out', args,{cwd:'./algo'}, (error, stdout, stderr) => {
		console.log(stdout);
		console.log(stderr);
		if (error) {
			res.status(500).end();
		}
		else {
			res.write(stdout);
			res.status(200).end();
		}
	});
});

module.exports = router;
