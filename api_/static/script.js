// Ensure DOM is loaded before initializing and binding functions
window.addEventListener('DOMContentLoaded', () => {
    initializeChart();
    drawDefaultGrid();
});

function calculateExponents(event) {
    event.preventDefault();

    const M = document.getElementById('M').value;
    const typeM = document.getElementById('TypeModulation').value;
    const SNR = document.getElementById('SNR').value;
    const R = document.getElementById('R').value;
    const N = document.getElementById('N').value;
    const resultDiv = document.getElementById('result');

    // Limpiar resultados anteriores
    resultDiv.innerHTML = "";
    resultDiv.classList.remove('show');

    fetch(`/exponents?M=${M}&typeM=${typeM}&SNR=${SNR}&R=${R}&N=${N}`)
        .then(response => {
            if (!response.ok) {
                throw new Error("Server response not OK");
            }
            return response.json();
        })
        .then(data => {
            resultDiv.innerHTML = `
                <p><strong>Probability error:</strong> ${data["Probabilidad de error"].toFixed(4)}</p>
                <p><strong>Exponents:</strong> ${data["Exponents"].toFixed(4)}</p>
                <p><strong>Optimal rho:</strong> ${data["rho óptima"].toFixed(4)}</p>
            `;
            resultDiv.classList.add('show');
        })
        .catch(error => {
            console.error("Error fetching exponents:", error);
            resultDiv.innerHTML = `<p style="color: red; font-weight: bold;">⚠️ Unable to process the data. Please verify your inputs.</p>`;
            resultDiv.classList.add('show');
        });
}


/* Plot Using the ENDPOINT */
function plotFromFunction() {
    const y = document.getElementById('yVar').value;
    const x = document.getElementById('xVar').value;
    const [min, max] = document.getElementById('xRange').value.split(',').map(Number);
    const points = Number(document.getElementById('points').value);
    const typeModulation = document.getElementById('funcTypeModulation').value;

    // Recoger todos los valores fijos
    const M = document.getElementById('fixedM').value;
    const SNR = document.getElementById('fixedSNR').value;
    const Rate = document.getElementById('fixedRate').value;
    const N = document.getElementById('fixedN').value;

    const inputs = { M, SNR, Rate, N };

    // Validar campos (excepto el que se escoge como X)
    const resultDiv = document.getElementById('plot-result');
    resultDiv.innerHTML = "";
    resultDiv.classList.remove('show');

    // Validar campos (excepto el que se escoge como X)
    if (isNaN(min) || isNaN(max) || min >= max) {
        resultDiv.innerHTML = `<p style="color: red; font-weight: bold;">⚠️ Please enter a valid range (min < max) for X axis.</p>`;
        resultDiv.classList.add('show');
        return;
    } 

    for (const [key, value] of Object.entries(inputs)) {
        if (key !== x && (value === '' || isNaN(parseFloat(value)))) {
            resultDiv.innerHTML = `<p style="color: red; font-weight: bold;">⚠️ Please enter a valid value for ${key}.</p>`;
            resultDiv.classList.add('show');
            return;
        }
    }

    const lineType = document.getElementById('lineType').value || '-';
    const color = document.getElementById('lineColor').value || 'steelblue';
    const plotType = document.getElementById('plotType').value;

    const payload = {
        y, x,
        rang_x: [min, max],
        points,
        typeModulation,
        M: parseFloat(M) || 0,
        SNR: parseFloat(SNR) || 0,
        Rate: parseFloat(Rate) || 0,
        N: parseFloat(N) || 0,
        color,
        lineType,
        plotType
    };

    console.log("Sending payload to /plot_function:", payload);

    document.getElementById('plot-result').innerHTML = "";
    document.getElementById('plot-result').classList.remove('show');

    fetch('/plot_function', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    })
    .then(response => {
        if (!response.ok) throw new Error("Error en /plot_function");
        return response.json();
    })
    .then(data => {
        console.log("Datos recibidos del backend:", data); 
        drawInteractivePlot(data.x, data.y, {
            color: color,
            lineType: lineType,
            plotType: plotType
        });
    })
    .catch(error => {
        console.error("Error plotting data", error);
        const resultDiv = document.getElementById('plot-result');
        resultDiv.innerHTML = `<p style="color: red; font-weight: bold;">⚠️ Unable to process the data. Please verify your inputs.</p>`;
        resultDiv.classList.add('show');
    });
}


/* Plot Manually */
function plotManually() {
    const xInput = document.getElementById('xValues').value.trim();
    const yInput = document.getElementById('yValues').value.trim();
    const color = document.getElementById('lineColor').value || 'steelblue';
    const plotType = document.getElementById('plotType').value;
    const lineType = document.getElementById('lineType').value || '-';

    const resultDiv = document.getElementById('plot-result');
    resultDiv.innerHTML = "";
    resultDiv.classList.remove('show');

    try {
        // Validar si se ingresaron valores
        if (!xInput || !yInput) {
            throw new Error("Missing input values.");
        }

        const x = xInput.split(',').map(str => Number(str.trim()));
        const y = yInput.split(',').map(str => Number(str.trim()));

        if (x.length !== y.length) {
            throw new Error("Mismatched array lengths.");
        }

        if (x.some(isNaN) || y.some(isNaN)) {
            throw new Error("Invalid number in inputs.");
        }

        // Ordenar por x para una gráfica coherente
        const sorted = x.map((val, i) => ({ x: val, y: y[i] }))
            .sort((a, b) => a.x - b.x);

        const xSorted = sorted.map(p => p.x);
        const ySorted = sorted.map(p => p.y);

        drawInteractivePlot(xSorted, ySorted, {
            color: color,
            lineType: lineType,
            plotType: plotType
        });

    } catch (error) {
        console.error("Error plotting manual data:", error);
        resultDiv.innerHTML = `<p style="color: red; font-weight: bold;">⚠️ Unable to process the data in order to plot it manually. Please verify your inputs.</p>`;
        resultDiv.classList.add('show');
    }
}

// Interactive multi-plot with zoom, grid, tooltip, overlay & removal
let activePlots = [];
let plotIdCounter = 1;

function initializeChart() {
    const margin = { top: 20, right: 30, bottom: 50, left: 60 };
    const width = 800;
    const height = 600;
    d3.select('#plot-output').html('');
    const container = d3.select('#plot-output')
        .append('div').attr('class','plot-container').style('position','relative');
    const svg = container.append('svg')
        .attr('viewBox', `0 0 ${width} ${height}`)
        .attr('preserveAspectRatio','xMidYMid meet')
        .style('width','100%').style('height','auto');
    svg.append('line')  // eje inferior
        .attr('x1', margin.left)
        .attr('x2', width - margin.right)
        .attr('y1', height - margin.bottom)
        .attr('y2', height - margin.bottom)
        .attr('stroke', 'black');
    
    svg.append('line')  // eje izquierdo
        .attr('x1', margin.left)
        .attr('x2', margin.left)
        .attr('y1', margin.top)
        .attr('y2', height - margin.bottom)
        .attr('stroke', 'black');
      
    window.__svg = svg;
    window.__g = svg.append('g').attr('class','main-group')
        .attr('transform',`translate(${margin.left},${margin.top})`);
    window.__innerWidth = width - margin.left - margin.right;
    window.__innerHeight = height - margin.top - margin.bottom;
    window.__xScale = d3.scaleLinear().range([0,window.__innerWidth]);
    window.__yScale = d3.scaleLinear().range([window.__innerHeight,0]);
    window.__gridX = window.__g.append('g').attr('class','grid-x')
        .attr('transform',`translate(0,${window.__innerHeight})`);
    window.__gridY = window.__g.append('g').attr('class','grid-y');
    window.__gX = window.__g.append('g').attr('class','axis x-axis')
        .attr('transform',`translate(0,${window.__innerHeight})`);
    window.__gY = window.__g.append('g').attr('class','axis y-axis');
    window.__content = window.__g.append('g').attr('class','content');
    window.__tooltip = d3.select('#plot-output').append('div').attr('class','tooltip')
        .style('position','absolute').style('background','rgba(0,0,0,0.75)')
        .style('color','white').style('padding','5px 10px')
        .style('border-radius','5px').style('pointer-events','none')
        .style('opacity',0);

    // Controls wrapper (centering)
    const controlsWrapper = d3.select('#plot-output')
        .append('div')
        .attr('id', 'plot-controls-wrapper')
        .style('display', 'flex')
        .style('justify-content', 'center')
        .style('width', '100%');

    const controls = controlsWrapper
        .append('div')
        .attr('id','plot-controls')
        .style('display','flex')
        .style('justify-content','center')
        .style('align-items','center')
        .style('gap','20px')
        .style('margin-top','12px')
        .style('flex-wrap','wrap');

    // Reset Zoom button
    controls.append('button')
        .attr('type','button')
        .attr('class','reset-zoom-button')
        .text('Reset Zoom')
        .on('click', resetZoom);

    // Clear all plots button
    controls.append('button')
        .attr('type','button')
        .attr('class','reset-zoom-button') // reuse same style
        .text('Clear All Plots')
        .on('click', () => {
            activePlots.length = 0; // clear array without reassignment
            renderAll();
            updatePlotListUI();
        });

    // Grid toggle
    controls.append('label').html('<input type="checkbox" id="toggleGrid" checked> Show grid');

    // Points toggle
    controls.append('label').html('<input type="checkbox" id="togglePoints"> Show points');

    // Zoom behavior
    window.__zoom = d3.zoom().scaleExtent([1,10]).on('zoom',zoomed);
    window.__svg.call(window.__zoom);

    // Grid and point toggles re-render without resetting zoom
    d3.select('#toggleGrid').on('change', () =>
        zoomed({ transform: d3.zoomTransform(window.__svg.node()) })
      );
    d3.select('#togglePoints').on('change',()=>zoomed(window.__lastZoomEvent || { transform: d3.zoomTransform(window.__svg.node()) }));
}


function resetZoom() {
    if(!activePlots.length) return;
    let maxR=-Infinity, target=null;
    activePlots.forEach(p=>{
        const yExt=d3.extent(p.y), rng=yExt[1]-yExt[0];
        if(rng>maxR){maxR=rng;target=p;}
    });
    if(!target) return;
    window.__xScale.domain(d3.extent(target.x));
    window.__yScale.domain(d3.extent(target.y));
    window.__svg.transition().duration(750).call(window.__zoom.transform,d3.zoomIdentity);
    setTimeout(() => zoomed({transform:d3.zoomIdentity}), 750);
}



function zoomed(event) {
    const t = event.transform;
    window.__lastZoomEvent = event;

    // 1) Rescala
    const newX = t.rescaleX(window.__xScale);
    const newY = t.rescaleY(window.__yScale);

    // 2) Ejes (siempre los dibujamos, pero quitamos el "domain" que dibuja el marco completo)
    window.__gX.call(d3.axisBottom(newX)).select('.domain').remove();
    window.__gY.call(d3.axisLeft(newY)).select('.domain').remove();

    // 3) Grid toggle: data‑binding de líneas en lugar de axisBottom/Left
    if (d3.select('#toggleGrid').property('checked')) {
        const xTicks = newX.ticks();
        const yTicks = newY.ticks();

        // verticales
        window.__gridX.selectAll('line')
            .data(xTicks)
            .join('line')
            .attr('x1', d => newX(d))
            .attr('x2', d => newX(d))
            .attr('y1', 0)
            .attr('y2', -window.__innerHeight)
            .attr('stroke', '#ddd')
            .attr('stroke-dasharray', '2,2');

        // horizontales
        window.__gridY.selectAll('line')
            .data(yTicks)
            .join('line')
            .attr('x1', 0)
            .attr('x2', window.__innerWidth)
            .attr('y1', d => newY(d))
            .attr('y2', d => newY(d))
            .attr('stroke', '#ddd')
            .attr('stroke-dasharray', '2,2');
    } else {
        // quitar todas las líneas cuando esté off
        window.__gridX.selectAll('line').remove();
        window.__gridY.selectAll('line').remove();
    }

    // 4) contenido (curvas, puntos, zoom, tooltips…)
    window.__content.attr('transform', t);
    const scaleFactor = 1 / t.k;
    d3.selectAll('g.points circle').attr('r', 4 * scaleFactor);
    d3.selectAll('path.line').attr('stroke-width', 2 * scaleFactor);
    const visible = d3.select('#togglePoints').property('checked') ? 'visible' : 'hidden';
    d3.selectAll('g.points circle').attr('visibility', visible);
}



function renderAll() {
    if (activePlots.length === 0) {
        if (window.__content) window.__content.selectAll('*').remove();
        // d3.select('#plot-controls-wrapper').style('display', 'none');
        drawDefaultGrid();
        return;
    }

    if (!window.__svg || !window.__content) return;

    d3.select('#plot-container').style('display', 'block');
    d3.select('#plot-controls-wrapper').style('display', 'flex');

    let allX = [], allY = [];
    activePlots.forEach(p => { allX = allX.concat(p.x); allY = allY.concat(p.y); });
    if (allX.length === 0) return;
    const xExtent = d3.extent(allX);
    const yExtent = d3.extent(allY);

    window.__xScale.domain(xExtent);
    window.__yScale.domain(yExtent);

    window.__svg.call(window.__zoom.transform, d3.zoomIdentity);

    zoomed({ transform: d3.zoomIdentity });

    const groups = window.__content.selectAll('.plot-group')
        .data(activePlots, d => d.plotId);

    const enterG = groups.enter()
        .append('g')
        .attr('class', d => `plot-group ${d.plotId}`);

    enterG.append('path').attr('class', 'line');
    enterG.append('g').attr('class', 'points');

    window.__content.selectAll('.plot-group').each(function(d) {
        const g = d3.select(this);
        const lineGen = d3.line()
            .curve(getCurve(d.lineType))
            .x((_, i) => window.__xScale(d.x[i]))
            .y((_, i) => window.__yScale(d.y[i]));

        g.select('path.line')
            .datum(d.y)
            .attr('fill', 'none')
            .attr('stroke', d.color)
            .attr('stroke-width', 2)
            .attr('d', lineGen);

        const pts = g.select('g.points').selectAll('circle')
            .data(d.y);
        pts.enter().append('circle');
        g.selectAll('g.points circle')
            .attr('r', 4)
            .attr('fill', d.color)
            .attr('cx', (_, i) => window.__xScale(d.x[i]))
            .attr('cy', (_, i) => window.__yScale(d.y[i]))
            .attr('visibility', d3.select('#togglePoints').property('checked') ? 'visible' : 'hidden')
            .on('mouseover', function(event, v) {
                const i = d.y.indexOf(v);
                window.__tooltip
                    .html(`x: ${d.x[i]}<br>y: ${v.toFixed(4)}`)
                    .style('left', (event.offsetX + 15) + 'px')
                    .style('top', (event.offsetY - 25) + 'px')
                    .style('opacity', 1);
            })
            .on('mouseout', () => window.__tooltip.style('opacity', 0));

        pts.exit().remove();
    });

    groups.exit().remove();
    updatePlotListUI();
    d3.select('#plot-controls-wrapper').style('display', 'flex');
}

function getCurve(type) {
    return { '-': d3.curveLinear, '--': d3.curveStep, 'o-': d3.curveBasis }[type] || d3.curveLinear;
}


function drawInteractivePlot(x, y, opts) {
    opts = opts || {};
    const plotId = `plot-${plotIdCounter++}`;
    const yLabel = document.getElementById('yVar')?.selectedOptions[0]?.text || `Y`;
    const xLabel = document.getElementById('xVar')?.selectedOptions[0]?.text || `X`;
    const label = `${yLabel} / ${xLabel}`;
    activePlots.push({ plotId, x, y, color: opts.color || 'steelblue', lineType: opts.lineType || '-', label });
    renderAll();
}


function updatePlotListUI() {
    let container = document.getElementById('plot-list');
    if (!container) {
        container = document.createElement('div');
        container.id = 'plot-list';
        container.style.marginTop = '20px';
        container.style.display = 'flex';
        container.style.flexDirection = 'column';
        container.style.alignItems = 'flex-start';
        container.style.paddingRight = '10px';
        container.style.boxSizing = 'border-box';
        container.style.maxWidth = '100%';
        document.getElementById('plot-output').appendChild(container);
    }
    container.innerHTML = '<h4>Active plots:</h4>';
    activePlots.forEach((p, i) => {
        const item = document.createElement('div');
        item.className = `legend-item ${p.plotId}`;
        item.style.margin = '5px 0';
        item.style.cursor = 'pointer';
        item.style.display = 'grid';
        item.style.gridTemplateColumns = '20px 1fr auto';
        item.style.alignItems = 'center';
        item.style.columnGap = '10px';
        item.style.transition = 'transform 0.3s ease';
        item.style.width = 'calc(100% - 20px)';
        item.style.boxSizing = 'border-box';
        item.style.paddingRight = '10px';

        const colorBox = document.createElement('span');
        colorBox.style.display = 'inline-block';
        colorBox.style.width = '15px';
        colorBox.style.height = '15px';
        colorBox.style.background = p.color;

        const textSpan = document.createElement('span');
        textSpan.textContent = p.label;
        textSpan.style.wordBreak = 'break-word';

        const btn = document.createElement('button');
        btn.textContent = '❌ Remove';
        btn.type = 'button';
        btn.style.marginRight = '10px';
        btn.onclick = () => removePlot(p.plotId);

        item.appendChild(colorBox);
        item.appendChild(textSpan);
        item.appendChild(btn);
        container.appendChild(item);

        item.addEventListener('mouseover', () => highlightPlot(p.plotId, true));
        item.addEventListener('mouseout', () => highlightPlot(p.plotId, false));
    });
}

function removePlot(plotId) {
    activePlots = activePlots.filter(p => p.plotId !== plotId);
    renderAll();
    updatePlotListUI();
    if (activePlots.length === 0 && window.__content) {
        window.__content.selectAll('*').remove();
        // d3.select('#plot-controls-wrapper').style('display', 'none');
        drawDefaultGrid();
    }
}

function highlightPlot(plotId, highlight) {
    const group = d3.select(`.plot-group.${plotId}`);
    if (!group.empty()) {
        group.select('path.line')
            .transition().duration(250)
            .attr('stroke-width', highlight ? 4 : 2)
            .attr('opacity', highlight ? 1 : 0.8);
        group.selectAll('circle')
            .transition().duration(250)
            .attr('r', highlight ? 6 : 4)
            .attr('opacity', highlight ? 1 : 0.8);
    }

    const legendItem = document.querySelector(`.legend-item.${plotId}`);
    if (legendItem) {
        legendItem.style.fontWeight = highlight ? 'bold' : 'normal';
        legendItem.style.transform = highlight ? 'scale(1.03)' : 'scale(1)';
    }
}

function drawDefaultGrid() {
    // Fija el dominio que quieras ver al cargar (por ejemplo de -10 a 10)
    window.__xScale.domain([-10, 10]);
    window.__yScale.domain([-10, 10]);
    // Llama a zoomed con identidad para dibujar ejes y cuadrícula
    zoomed({ transform: d3.zoomIdentity });
}



/* Hide and Show Manual Inputs */
function toggleManualInputs() {
    const manual = document.getElementById('manual-section');
    const btn = document.getElementById('toggleManualBtn');
    const visible = manual.style.display === 'block';
    manual.style.display = visible ? 'none' : 'block';
    btn.textContent = visible ? 'Add manually' : 'Hide manual inputs';
}

/* Validate data for X selection */
(function setupValidation() {
    const xVarSelect = document.getElementById('xVar');
    const inputFields = {
        M: document.getElementById('fixedM'),
        SNR: document.getElementById('fixedSNR'),
        Rate: document.getElementById('fixedRate'),
        N: document.getElementById('fixedN')
    };
    xVarSelect.addEventListener('change', () => {
        const sel = xVarSelect.value;
        for (let k in inputFields) {
            if (k === sel) {
                inputFields[k].value = '';
                inputFields[k].disabled = true;
                inputFields[k].placeholder = '(Set by X)';
            } else {
                inputFields[k].disabled = false;
                inputFields[k].placeholder = k + '...';
            }
        }
    });
    window.addEventListener('DOMContentLoaded', () => xVarSelect.dispatchEvent(new Event('change')));
})();
