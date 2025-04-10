const d3 = require('d3-node').d3;
const { JSDOM } = require('jsdom');

function generatePlot(params) {
    const { y, x = [], rang_x, color = 'steelblue', lineType = '-', plotType = 'linear' } = params;
    const rang_y = d3.extent(y);

    const dom = new JSDOM();
    const document = dom.window.document;

    const svg = d3.select(document.body)
        .append('svg')
        .attr('width', 800)
        .attr('height', 600)
        .attr('xmlns', 'http://www.w3.org/2000/svg');

    const xScale = createScale(x, rang_x, plotType, 50, 750);
    const yScale = createScale(y, rang_y, plotType.includes('logy') ? 'log' : 'linear', 550, 50);

    svg.append('g')
        .attr('transform', `translate(0,550)`)
        .call(d3.axisBottom(xScale));

    svg.append('g')
        .attr('transform', 'translate(50,0)')
        .call(d3.axisLeft(yScale));

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
    const scale = scaleType === 'log' ? d3.scaleLog().clamp(true) : d3.scaleLinear();
    return scale.domain(range || extent).range([rangeStart, rangeEnd]);
}

function getCurve(lineType) {
    const curves = {
        '-': d3.curveLinear,
        '--': d3.curveStep,
        'o-': d3.curveBasis
    };
    return curves[lineType] || d3.curveLinear;
}

module.exports = generatePlot;
