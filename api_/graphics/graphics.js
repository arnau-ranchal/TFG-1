const d3 = require('d3-node').d3; 
const { JSDOM } = require('jsdom');

function generatePlot(params) {
    const { y, x = [], rang_x, color = 'steelblue', lineType = '-', plotType = 'linear' } = params;

    // Calcular automáticamente rang_y a partir de los valores de y
    const rang_y = d3.extent(y);

    const dom = new JSDOM();
    const document = dom.window.document;

    const svg = d3.select(document.body)
        .append('svg')
        .attr('width', 800)
        .attr('height', 600)
        .attr('xmlns', 'http://www.w3.org/2000/svg');

    // Configuración de escalas según tipo de plot
    const xScale = createScale(x, rang_x, plotType, 50, 750);
    const yScale = createScale(y, rang_y, plotType.includes('logy') ? 'log' : 'linear', 550, 50);

    // Ejes
    svg.append('g')
        .attr('transform', `translate(0,550)`)
        .call(d3.axisBottom(xScale));

    svg.append('g')
        .attr('transform', 'translate(50,0)')
        .call(d3.axisLeft(yScale));

    // Línea principal
    const lineGenerator = d3.line()
        .x((d, i) => xScale(x[i] || i))
        .y(d => yScale(d))
        .curve(getCurve(lineType));

    svg.append('path')
        .datum(y)
        .attr('fill', 'none')
        .attr('stroke', color)
        .attr('stroke-width', 2)
        .attr('d', lineGenerator);

    return document.body.innerHTML;
}

function createScale(data, range, scaleType, rangeStart, rangeEnd) {
    const extent = data.length > 0 ? d3.extent(data) : [0, 1];
    let scale;

    switch(scaleType) {
        case 'log':
            scale = d3.scaleLog().clamp(true);
            break;
        default:
            scale = d3.scaleLinear();
    }

    return scale
        .domain(range || extent)
        .range([rangeStart, rangeEnd]);
}

function getCurve(lineType) {
    const curves = {
        '-': d3.curveLinear,
        '--': d3.curveStep,
        'o-': d3.curveBasis
    };
    return curves[lineType] || d3.curveLinear;
}

// Manejo de entrada/salida
process.stdin.setEncoding('utf8');
let input = '';

process.stdin.on('data', chunk => input += chunk);
process.stdin.on('end', () => {
    try {
        const params = JSON.parse(input);
        console.log(generatePlot(params));
    } catch (error) {
        console.error(`ERROR: ${error.message}`);
        process.exit(1);
    }
});
