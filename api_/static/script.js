function calculateExponents(event) {
    event.preventDefault();
    const M = document.getElementById('M').value;
    const typeM = document.getElementById('TypeModulation').value;
    const SNR = document.getElementById('SNR').value;
    const R = document.getElementById('R').value;
    const N = document.getElementById('N').value;
    const resultDiv = document.getElementById('result');

    fetch(`/exponents?M=${M}&typeM=${typeM}&SNR=${SNR}&R=${R}&N=${N}`)
        .then(response => response.json())
        .then(data => {
            resultDiv.innerHTML = `
                <p><strong>Probability error:</strong> ${data["Probabilidad de error"].toFixed(4)}</p>
                <p><strong>Exponents:</strong> ${data["Exponents"].toFixed(4)}</p>
                <p><strong>Optimal rho:</strong> ${data["rho óptima"].toFixed(4)}</p>
            `;
            resultDiv.classList.add('show');
        })
        .catch(error => {
            resultDiv.innerHTML = `<p style="color: red;">Error: ${error}</p>`;
            resultDiv.classList.add('show');
        });
}

function plotManually() {
    const x = document.getElementById('xValues').value.split(',').map(Number);
    const y = document.getElementById('yValues').value.split(',').map(Number);
    const color = document.getElementById('lineColor').value || 'steelblue';
    const plotType = document.getElementById('plotType').value;
    const lineType = document.getElementById('lineType').value || '-';

    const rang_x = [Math.min(...x), Math.max(...x)];

    const payload = { x, y, rang_x, color, lineType, plotType };

    fetch('/plot_manually', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    })
    .then(response => response.text())
    .then(svg => {
        document.getElementById('plot-output').innerHTML = svg;
    })
    .catch(console.error);
}

function plotFromFunction() {
    const y = document.getElementById('yVar').value;
    const x = document.getElementById('xVar').value;
    const [min, max] = document.getElementById('xRange').value.split(',').map(Number);
    const points = Number(document.getElementById('points').value);
    const typeModulation = document.getElementById('funcTypeModulation').value;
    const M = parseFloat(document.getElementById('fixedM').value);
    const SNR = parseFloat(document.getElementById('fixedSNR').value);
    const Rate = parseFloat(document.getElementById('fixedRate').value);
    const N = parseFloat(document.getElementById('fixedN').value);
    const lineType = document.getElementById('lineType').value || '-';
    const color = document.getElementById('lineColor').value || 'steelblue';
    const plotType = document.getElementById('plotType').value;

    // Validación de inputs (encara que un haura de poder estar buit per defecte...)
    if (
        isNaN(M) || isNaN(SNR) || isNaN(Rate) || isNaN(N) ||
        isNaN(min) || isNaN(max) || isNaN(points) ||
        !y || !x || !typeModulation
    ) {
        alert("Please fill in all required numeric and selection fields correctly.");
        return;
    }

    const payload = {
        y, x,
        rang_x: [min, max],
        points,
        typeModulation,
        M, SNR, Rate, N,
        color,
        lineType,
        plotType
    };

    console.log("Sending payload to /plot_function:", payload);

    fetch('/plot_function', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    })
    .then(response => {
        if (!response.ok) {
            throw new Error("Error en /plot_function");
        }
        return response.json();
    })
    .then(data => {
        console.log("Datos recibidos del backend:", data); 
        drawInteractivePlot(data.x, data.y, {
            color: color,         // color obtenido del input
            lineType: lineType,   // línea, dash o curva, según lo escogido
            plotType: plotType    // "linear" o "log", según el input
        });
    })
    .catch(error => {
        console.error("Error plotting data", error);
    });
}


function drawInteractivePlot(x, y, opts) {
    opts = opts || {};
    const color = opts.color || "steelblue";
    const lineType = opts.lineType || "-";
    const plotType = opts.plotType || "linear";

    d3.select("#plot-output").html("");

    const margin = { top: 20, right: 30, bottom: 50, left: 60 };
    const width = 800;
    const height = 600;

    const container = d3.select("#plot-output")
        .append("div")
        .style("position", "relative")
        .style("overflow", "hidden")
        .style("max-width", "100%");

    const svg = container
        .append("svg")
        .attr("viewBox", `0 0 ${width} ${height}`)
        .attr("preserveAspectRatio", "xMidYMid meet")
        .style("width", "100%")
        .style("height", "auto");

    const g = svg.append("g")
        .attr("transform", `translate(${margin.left},${margin.top})`);

    const innerWidth = width - margin.left - margin.right;
    const innerHeight = height - margin.top - margin.bottom;

    const xExtent = d3.extent(x);
    const yExtent = d3.extent(y);
    if (xExtent[0] === xExtent[1]) xExtent[1] += 1;
    if (yExtent[0] === yExtent[1]) yExtent[1] += 1;

    let xScale = plotType === "log"
        ? d3.scaleLog().domain([Math.max(xExtent[0], 0.1), xExtent[1]]).range([0, innerWidth])
        : d3.scaleLinear().domain(xExtent).range([0, innerWidth]);

    let yScale = plotType === "log"
        ? d3.scaleLog().domain([Math.max(yExtent[0], 0.1), yExtent[1]]).range([innerHeight, 0])
        : d3.scaleLinear().domain(yExtent).range([innerHeight, 0]);

    const gX = g.append("g")
        .attr("transform", `translate(0,${innerHeight})`)
        .call(d3.axisBottom(xScale));

    const gY = g.append("g").call(d3.axisLeft(yScale));

    const gGrid = g.append("g").attr("class", "grid-group");
    const content = g.append("g").attr("class", "content");

    function getCurve(type) {
        const curves = {
            '-': d3.curveLinear,
            '--': d3.curveStep,
            'o-': d3.curveBasis
        };
        return curves[type] || d3.curveLinear;
    }

    function drawGrid(xS, yS) {
        gGrid.selectAll("*").remove();

        gGrid.selectAll(".y-grid")
            .data(yS.ticks(10))
            .enter()
            .append("line")
            .attr("class", "grid-line")
            .attr("x1", 0)
            .attr("x2", innerWidth)
            .attr("y1", d => yS(d))
            .attr("y2", d => yS(d))
            .attr("stroke", "#ccc")
            .attr("stroke-dasharray", "2,2");

        gGrid.selectAll(".x-grid")
            .data(xS.ticks(10))
            .enter()
            .append("line")
            .attr("class", "grid-line")
            .attr("y1", 0)
            .attr("y2", innerHeight)
            .attr("x1", d => xS(d))
            .attr("x2", d => xS(d))
            .attr("stroke", "#ccc")
            .attr("stroke-dasharray", "2,2");
    }

    const line = d3.line()
        .curve(getCurve(lineType))
        .x((d, i) => xScale(x[i]))
        .y(d => yScale(d));

    content.append("path")
        .datum(y)
        .attr("fill", "none")
        .attr("stroke", color)
        .attr("stroke-width", 2)
        .attr("d", line);

    const tooltip = d3.select("#plot-output")
        .append("div")
        .attr("class", "tooltip")
        .style("position", "absolute")
        .style("background", "rgba(0,0,0,0.75)")
        .style("color", "white")
        .style("padding", "5px 10px")
        .style("border-radius", "5px")
        .style("pointer-events", "none")
        .style("opacity", 0);

    const circles = content.selectAll("circle")
        .data(y)
        .enter()
        .append("circle")
        .attr("cx", (d, i) => xScale(x[i]))
        .attr("cy", d => yScale(d))
        .attr("r", 4)
        .attr("fill", color)
        .attr("pointer-events", "visible")
        .on("mouseover", function(event, d) {
            const i = y.indexOf(d);
            tooltip
                .html(`x: ${x[i]}<br>y: ${d.toFixed(4)}`)
                .style("left", (event.offsetX + 15) + "px")
                .style("top", (event.offsetY - 25) + "px")
                .style("opacity", 1);
        })
        .on("mouseout", () => tooltip.style("opacity", 0));

    // 👇 Crear los controles ANTES de leer su valor
    const controlsContainer = d3.select("#plot-output")
        .append("div")
        .attr("id", "plot-controls")
        .style("display", "flex")
        .style("justify-content", "center")
        .style("align-items", "center")
        .style("gap", "20px")
        .style("margin-top", "12px")
        .style("flex-wrap", "wrap");

    controlsContainer.append("button")
        .attr("type", "button")
        .attr("class", "reset-zoom-button")
        .text("Reset Zoom")
        .style("margin", "0 10px")
        .on("click", () => svg.transition().duration(500).call(zoom.transform, d3.zoomIdentity));

    controlsContainer.append("label")
        .style("margin-left", "15px")
        .html(`
            <input type="checkbox" id="togglePoints" checked>
            Show points
        `);

    controlsContainer.append("label")
        .style("margin-left", "15px")
        .html(`
            <input type="checkbox" id="toggleGrid">
            Show grid
        `);

    // 👇 Ahora que los checkboxes existen, ya podemos leer su estado
    const showPointsChecked = d3.select("#togglePoints").property("checked");
    circles.style("visibility", showPointsChecked ? "visible" : "hidden");

    d3.select("#togglePoints").on("change", function () {
        const visible = this.checked ? "visible" : "hidden";
        content.selectAll("circle").style("visibility", visible);
    });

    d3.select("#toggleGrid").on("change", function () {
        if (this.checked) {
            drawGrid(xScale, yScale);
        } else {
            gGrid.selectAll("*").remove();
        }
    });

    const zoom = d3.zoom()
        .scaleExtent([1, 10])
        .on("zoom", event => {
            const transform = event.transform;
            const newX = transform.rescaleX(xScale);
            const newY = transform.rescaleY(yScale);

            gX.call(d3.axisBottom(newX));
            gY.call(d3.axisLeft(newY));

            content.selectAll("path")
                .attr("d", d3.line()
                    .curve(getCurve(lineType))
                    .x((d, i) => newX(x[i]))
                    .y(d => newY(d)));

            content.selectAll("circle")
                .attr("cx", (d, i) => newX(x[i]))
                .attr("cy", d => newY(d));

            if (d3.select("#toggleGrid").property("checked")) {
                drawGrid(newX, newY);
            }
        });

    svg.call(zoom);
}


/* Hide and Show Manual Inputs */
function toggleManualInputs() {
    const manual = document.getElementById("manual-section");
    const btn = document.getElementById("toggleManualBtn");
    const visible = manual.style.display === "block";
    manual.style.display = visible ? "none" : "block";
    btn.textContent = visible ? "Add inputs manually" : "Hide manual inputs";
}