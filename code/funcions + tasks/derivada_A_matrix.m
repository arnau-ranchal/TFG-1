%% F0 + F0' + F0'' matrices
function derivada = derivada_A_matrix(Q, PI_, G, rho, R)
    logG = log(G);
    G1 = exp(-rho/(1 + rho)*logG);
    G2 = exp(1/(1 + rho)*logG);
    qg2 = Q.'*G2;
    logqg2 = log(qg2);
    qg2rho = exp(rho*logqg2);
    pig1 = PI_.*G1;

    m = Q.'*pig1*(qg2rho.');
    mp = Q.'*pig1*((qg2rho.*logqg2).') - 1/(1 + rho) * Q.'*(pig1.*logG)*(qg2rho.');
    %m2p = Q.'*pig1*((qg2rho.*logqg2.*logqg2).') - 2/(1 + rho) * Q.'*(pig1.*logG) * ((qg2rho.*logqg2).') + 1/(1 + rho)^2 * Q.'*(pig1.*logG.*logG)*(qg2rho.');
        
    m = (1/pi)*m; mp = (1/pi)*mp; %m2p = (1/pi)*m2p;

    derivada = - mp /(m * log(2)) - R;

end
