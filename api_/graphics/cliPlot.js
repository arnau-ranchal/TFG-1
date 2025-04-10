const generatePlot = require('./generatePlot');

process.stdin.setEncoding('utf8');
let input = '';

process.stdin.on('data', chunk => input += chunk);
process.stdin.on('end', () => {
    try {
        const params = JSON.parse(input);
        const svg = generatePlot(params);
        console.log(svg);
    } catch (error) {
        console.error(`ERROR: ${error.message}`);
        process.exit(1);
    }
});
