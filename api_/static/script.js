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

    // Neteja resultats anteriors
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

    // Recull els valors fixos
    const M = document.getElementById('fixedM').value;
    const SNR = document.getElementById('fixedSNR').value;
    const Rate = document.getElementById('fixedRate').value;
    const N = document.getElementById('fixedN').value;

    const inputs = { M, SNR, Rate, N };

    // Validar inputs
    const resultDiv = document.getElementById('plot-result');
    resultDiv.innerHTML = "";
    resultDiv.classList.remove('show');

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

    /* console.log("Sending payload to /plot_function:", payload); */

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
    const xInputEl = document.getElementById('xValues');
    const yInputEl = document.getElementById('yValues');
    const xInput = xInputEl.value.trim();
    const yInput = yInputEl.value.trim();
    const color = document.getElementById('lineColor').value || 'steelblue';
    const plotType = document.getElementById('plotType').value;
    const lineType = document.getElementById('lineType').value || '-';
    //const manualTitle = document.getElementById('manualTitle').value || 'Manual Graph';
    const manualTitle = document.getElementById('manualTitle').value.trim();

    const resultDiv = document.getElementById('plot-result');
    resultDiv.innerHTML = "";
    resultDiv.classList.remove('show');

    // Reset styles
    xInputEl.classList.remove('input-error');
    yInputEl.classList.remove('input-error');

    try {
        if (!xInput || !yInput) {
            if (!xInput) xInputEl.classList.add('input-error');
            if (!yInput) yInputEl.classList.add('input-error');
            throw new Error("Missing input values.");
        }

        const x = xInput.split(',').map(str => Number(str.trim()));
        const y = yInput.split(',').map(str => Number(str.trim()));

        if (x.length !== y.length) {
            xInputEl.classList.add('input-error');
            yInputEl.classList.add('input-error');
            throw new Error("Mismatched array lengths.");
        }

        if (x.some(isNaN) || y.some(isNaN)) {
            xInputEl.classList.add('input-error');
            yInputEl.classList.add('input-error');
            throw new Error("Invalid number in inputs.");
        }

        const sorted = x.map((val, i) => ({ x: val, y: y[i] }))
            .sort((a, b) => a.x - b.x);

        const xSorted = sorted.map(p => p.x);
        const ySorted = sorted.map(p => p.y);

        drawInteractivePlot(xSorted, ySorted, {
            color: color,
            lineType: lineType,
            plotType: plotType,
            label: manualTitle   // si es cadena vacía, lo ignoraremos más abajo
        });

    } catch (error) {
        console.error("Error plotting manual data:", error);
        resultDiv.innerHTML = `<p style="color: red; font-weight: bold;">⚠️ Unable to process the data. Please check your input format.</p>`;
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
    svg.append('line')  // eix inferior
        .attr('x1', margin.left)
        .attr('x2', width - margin.right)
        .attr('y1', height - margin.bottom)
        .attr('y2', height - margin.bottom)
        .attr('stroke', 'black');
    
    svg.append('line')  // eix esquerre
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

    // 2) Eixos
    window.__gX.call(d3.axisBottom(newX)).select('.domain').remove();
    window.__gY.call(d3.axisLeft(newY)).select('.domain').remove();

    // 3) Grid toggle
    if (d3.select('#toggleGrid').property('checked')) {
        const xTicks = newX.ticks();
        const yTicks = newY.ticks();

        // verticals
        window.__gridX.selectAll('line')
            .data(xTicks)
            .join('line')
            .attr('x1', d => newX(d))
            .attr('x2', d => newX(d))
            .attr('y1', 0)
            .attr('y2', -window.__innerHeight)
            .attr('stroke', '#ddd')
            .attr('stroke-dasharray', '2,2');

        // horitzontals
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
        // treure totes les lineas cuan estigui off
        window.__gridX.selectAll('line').remove();
        window.__gridY.selectAll('line').remove();
    }

    // 4) Contingut (curvas, puntos, zoom, tooltips…)
    window.__content.attr('transform', t);
    const scaleFactor = 1 / t.k;
    d3.selectAll('g.points circle').attr('r', 4 * scaleFactor);
    d3.selectAll('path.line').attr('stroke-width', 2 * scaleFactor);
    const visible = d3.select('#togglePoints').property('checked') ? 'visible' : 'hidden';
    d3.selectAll('g.points circle').attr('visibility', visible);
}



function renderAll() {
    if (!window.__svg || !window.__content) return;

    if (activePlots.length === 0) {
        window.__content.selectAll('*').remove();
        drawDefaultGrid();
        return;
    }

    d3.select('#plot-container').style('display', 'block');
    d3.select('#plot-controls-wrapper').style('display', 'flex');

    let allX = [], allY = [], allLog = false;
    activePlots.forEach(p => {
        allX = allX.concat(p.x);
        allY = allY.concat(p.y);
        if (p.plotType === 'log') allLog = true;
    });

    if (allX.length === 0) return;

    const xExtent = d3.extent(allX);
    const yExtent = d3.extent(allY);

    const xMin = allLog ? Math.max(xExtent[0], 1e-6) : xExtent[0];
    const yMin = allLog ? Math.max(yExtent[0], 1e-6) : yExtent[0];

    window.__xScale = (allLog ? d3.scaleLog() : d3.scaleLinear()).range([0, window.__innerWidth]);
    window.__yScale = (allLog ? d3.scaleLog() : d3.scaleLinear()).range([window.__innerHeight, 0]);
    window.__xScale.domain([xMin, xExtent[1]]);
    window.__yScale.domain([yMin, yExtent[1]]);

    window.__svg.call(window.__zoom.transform, d3.zoomIdentity);
    setTimeout(() => zoomed({ transform: d3.zoomIdentity }), 0);

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
            .curve(d3.curveLinear)
            .x((_, i) => window.__xScale(d.x[i]))
            .y((_, i) => window.__yScale(d.y[i]));

        const dashMap = {
            'solid': '',
            'dashed': '6,4',
            'dotted': '2,4',
            'dot-dash': '4,2,2,2'
        };

        g.select('path.line')
            .datum(d.y)
            .attr('fill', 'none')
            .attr('stroke', d.color)
            .attr('stroke-width', 2)
            .attr('stroke-dasharray', dashMap[d.dashStyle] || '')
            .attr('d', lineGen);

        const pts = g.select('g.points').selectAll('circle')
            .data(d.x.map((xVal, i) => ({ x: xVal, y: d.y[i] })));

        pts.enter().append('circle')
            .merge(pts)
            .attr('r', 4)
            .attr('fill', d.color)
            .attr('cx', d => window.__xScale(d.x))
            .attr('cy', d => window.__yScale(d.y))
            .attr('visibility', d3.select('#togglePoints').property('checked') ? 'visible' : 'hidden')
            .on('mouseover', function(event, d) {
                window.__tooltip
                    .html(`x: ${d.x}<br>y: ${d.y.toFixed(4)}`)
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


function drawInteractivePlot(x, y, opts) {
    opts = opts || {};
    const plotId = `plot-${plotIdCounter++}`;
    // Generar el label: si nos llega opts.label lo usamos, si no, el default de From Function
    let label;
    if (opts.label && opts.label.length > 0) {
      label = opts.label;
    } else {
      const yLabel = document.getElementById('yVar')?.selectedOptions[0]?.text || 'Y';
      const xLabel = document.getElementById('xVar')?.selectedOptions[0]?.text || 'X';
      label = `${yLabel} / ${xLabel}`;
    }


    const color = opts.color || 'steelblue';
    const dashStyle = opts.lineType || 'solid';
    const plotType = opts.plotType || 'linear';

    let xCopy = [...x];
    let yCopy = [...y];

    // Filtrar ceros o negativos si es log
    if (plotType === 'log') {
        const filtered = xCopy.map((val, i) => ({ x: val, y: yCopy[i] }))
            .filter(p => p.x > 0 && p.y > 0);
        xCopy = filtered.map(p => p.x);
        yCopy = filtered.map(p => p.y);
    }

    activePlots.push({ plotId, x: xCopy, y: yCopy, color, dashStyle, label, plotType });
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

    activePlots.forEach(p => {
        const item = document.createElement('div');
        item.className = `legend-item ${p.plotId}`;
        item.dataset.plotId = p.plotId;                  // ① guardamos el id
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

        // colorBox (igual que antes)…
        const colorBox = document.createElement('span');
        colorBox.style.display = 'inline-block';
        colorBox.style.width = '15px';
        colorBox.style.height = '15px';
        colorBox.style.background = p.color;

        // Aquí va el texto editable
        const textSpan = document.createElement('span');
        textSpan.textContent = p.label;
        textSpan.style.wordBreak = 'break-word';
        textSpan.addEventListener('dblclick', () => {
            // ② crear input inplace
            const input = document.createElement('input');
            input.type = 'text';
            input.value = p.label;
            input.style.width = '100%';
            // si pulsa Enter, hacemos blur para disparar el commit
            input.addEventListener('keydown', e => {
                if (e.key === 'Enter') input.blur();
            });
            // al perder foco, actualizo label y reconstruyo UI
            input.addEventListener('blur', () => {
                const newLabel = input.value.trim();
                if (newLabel) p.label = newLabel;
                updatePlotListUI();
            });
            textSpan.replaceWith(input);
            input.focus();
        });

        // botón de eliminar (igual que antes)…
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
    const removedPlot = activePlots.find(p => p.plotId === plotId);
    activePlots = activePlots.filter(p => p.plotId !== plotId);
    renderAll();
    updatePlotListUI();

    if (activePlots.length === 0 && window.__content) {
        window.__content.selectAll('*').remove();

        // Si se eliminó un gráfico logarítmico, forzamos la escala log
        drawDefaultGrid(removedPlot?.plotType === 'log');
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

function drawDefaultGrid(forceLog = false) {
    const useLog = forceLog;

    window.__xScale = (useLog ? d3.scaleLog() : d3.scaleLinear()).range([0, window.__innerWidth]);
    window.__yScale = (useLog ? d3.scaleLog() : d3.scaleLinear()).range([window.__innerHeight, 0]);

    const defaultMin = useLog ? 1e-2 : -10;
    const defaultMax = 10;

    window.__xScale.domain([defaultMin, defaultMax]);
    window.__yScale.domain([defaultMin, defaultMax]);

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

// ------------------------ CHATBOT SCRIPT -----------------------------------
function sendMessage() {
    const input = document.getElementById('chat-input');
    const msg = input.value.trim();
    if (!msg) return;

    const chatBox = document.getElementById('chat-messages');

    const userMsg = document.createElement('div');
    userMsg.className = 'chat-bubble user';
    userMsg.textContent = msg + "🧑‍💻 ";
    chatBox.appendChild(userMsg);

    input.value = '';
    chatBox.scrollTop = chatBox.scrollHeight;

    fetch('/chatbot', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ message: msg })
    })
    .then(res => res.json())
    .then(data => {
        const botMsg = document.createElement('div');
        botMsg.className = 'chat-bubble bot';
        botMsg.textContent = "🤖 " + data.response;
        chatBox.appendChild(botMsg);
        chatBox.scrollTop = chatBox.scrollHeight;
    })
    .catch(err => {
        const errMsg = document.createElement('div');
        errMsg.className = 'chat-bubble error';
        errMsg.textContent = "⚠️ Error: " + err.message;
        chatBox.appendChild(errMsg);
        chatBox.scrollTop = chatBox.scrollHeight;
    });
}

function clearChat() {
    const chatBox = document.getElementById('chat-messages');
    chatBox.innerHTML = ''; // Borra todo el historial de mensajes
}

