%% E0
function result = E0_matrix(Q, PI_, G, rho)
    logG = log(G);
    G1 = exp(-rho/(1 + rho)*logG);
    G2 = exp(1/(1 + rho)*logG);

    qg2rho = exp(rho*log(Q.'*G2));

    m = Q.'*(PI_.*G1)*(qg2rho.');
    result = -log2((1/pi)*m);
end