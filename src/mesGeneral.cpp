#include <RcppArmadillo.h>
#include <iostream>
#include <cmath>
#include "mesGeneral.h"
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp;


/* # Function returns multiplicative or additive error for scalar */
double errorf(double const &yact, double &yfit, char const &E){
    if(E=='A'){
        return yact - yfit;
    }
    else if(E=='D'){
        // This is a logistic additive error
        double yProb;
        if(yfit > 500){
            if(yact==1){
                yProb = 1;
            }
            else{
                return(-yfit);
            }
        }
        else if(yfit < -500){
            if(yact==0){
                yProb = 0;
            }
            else{
                return(-yfit);
            }
        }
        else{
            yProb = exp(yfit) / (1 + exp(yfit));
        }
        return log((1 + yact - yProb)/(1 - yact + yProb));
    }
    else if(E=='L'){
        // This is a logistic multiplicative error
        // double yProb = yfit / (1 + yfit);
        return ((1 + yact + yact * yfit)/(1 - yact - yact * yfit + 2 * yfit) - 1);
    }
    else{
        if((yact==0) & (yfit==0)){
            return 0;
        }
        else if((yact!=0) & (yfit==0)){
            return R_PosInf;
        }
        else{
            return (yact - yfit) / yfit;
        }
    }
}

/* # Function is needed to estimate the correct error for ETS when multisteps model selection with r(matvt) is sorted out. */
arma::mat errorvf(arma::mat yact, arma::mat yfit, char const &E){
    if(E=='A'){
        return yact - yfit;
    }
    else if(E=='D'){
        // This is an additive logistic error
        yfit = clamp(yfit, -500,500);
        yfit = exp(yfit) / (1 + exp(yfit));

        return log((1 + yact - yfit)/(1 - yact + yfit));
    }
    else if(E=='L'){
        // This is a multiplicative logistic error
        // yfit = yfit / (1 + yfit);
        return ((1 + yact + yact * yfit)/(1 - yact - yact * yfit + 2 * yfit) - 1);
    }
    else{
        yfit.elem(find(yfit==0)).fill(1e-100);
        return (yact - yfit) / yfit;
    }
}

/* # initparams - function that initialises the basic parameters of ETS */
// [[Rcpp::export]]
RcppExport SEXP initparams(SEXP Etype, SEXP Ttype, SEXP Stype, SEXP datafreq, SEXP obsR, SEXP obsallR, SEXP yt,
                           SEXP damped, SEXP phi, SEXP smoothingparameters, SEXP initialstates, SEXP seasonalcoefs){

    char E = as<char>(Etype);
    char T = as<char>(Ttype);
    char S = as<char>(Stype);
    int freq = as<int>(datafreq);
    int obs = as<int>(obsR);
    int obsall = as<int>(obsallR);
    NumericMatrix yt_n(yt);
    arma::mat vecYt(yt_n.begin(), yt_n.nrow(), yt_n.ncol(), false);
    bool damping = as<bool>(damped);
    double phivalue;
    if(!Rf_isNull(phi)){
        phivalue = as<double>(phi);
    }

    NumericMatrix smoothingparam(smoothingparameters);
    arma::mat persistence(smoothingparam.begin(), smoothingparam.nrow(), smoothingparam.ncol(), false);
    NumericMatrix initials(initialstates);
    arma::mat initial(initials.begin(), initials.nrow(), initials.ncol(), false);
    NumericMatrix seasonalc(seasonalcoefs);
    arma::mat seascoef(seasonalc.begin(), seasonalc.nrow(), seasonalc.ncol(), false);

    unsigned int ncomponents = 1;
    int lagsModelMax = 1;
    arma::vec lagsModel(3, arma::fill::ones);

/* # Define the number of components */
    if(T!='N'){
        ncomponents += 1;
        if(S!='N'){
            ncomponents += 1;
            lagsModelMax = freq;
            lagsModel(2) = freq;
        }
        else{
            lagsModel.resize(2);
        }
    }
    else{
/* # Define the number of components and model frequency */
        if(S!='N'){
            ncomponents += 1;
            lagsModelMax = freq;
            lagsModel(1) = freq;
            lagsModel.resize(2);
        }
        else{
            lagsModel.resize(1);
        }
    }

    arma::mat matrixVt(std::max(obs + 2*lagsModelMax, obsall + lagsModelMax), ncomponents, arma::fill::ones);
    arma::vec vecG(ncomponents, arma::fill::zeros);
    bool estimphi = TRUE;

// # Define the initial states for level and trend components
    switch(T){
    case 'N':
        matrixVt.submat(0,0,lagsModelMax-1,0).each_row() = initial.submat(0,0,0,0);
    break;
    case 'A':
        matrixVt.submat(0,0,lagsModelMax-1,1).each_row() = initial.submat(0,0,0,1);
    break;
// # The initial matrix is filled with ones, that is why we don't need to fill in initial trend
    case 'M':
        matrixVt.submat(0,0,lagsModelMax-1,1).each_row() = initial.submat(0,2,0,3);
    break;
    }

/* # Define the initial states for seasonal component */
    switch(S){
    case 'A':
        matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1) = seascoef.col(0);
    break;
    case 'M':
        matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1) = seascoef.col(1);
    break;
    }

//    matrixVt.resize(obs+lagsModelMax, ncomponents);

    if(persistence.n_rows < ncomponents){
        if((E=='M') | (T=='M') | (S=='M')){
            vecG = persistence.submat(0,1,persistence.n_rows-1,1);
        }
        else{
            vecG = persistence.submat(0,0,persistence.n_rows-1,0);
        }
    }
    else{
        if((E=='M') | (T=='M') | (S=='M')){
            vecG = persistence.submat(0,1,ncomponents-1,1);
        }
        else{
            vecG = persistence.submat(0,0,ncomponents-1,0);
        }
    }

    if(Rf_isNull(phi)){
        if(damping==TRUE){
            phivalue = 0.95;
        }
        else{
            phivalue = 1.0;
            estimphi = FALSE;
        }
    }
    else{
        if(damping==FALSE){
            phivalue = 1.0;
        }
        estimphi = FALSE;
    }

    return wrap(List::create(Named("nComponents") = ncomponents, Named("lagsModelMax") = lagsModelMax, Named("lagsModel") = lagsModel,
                             Named("matvt") = matrixVt, Named("vecg") = vecG, Named("phiEstimate") = estimphi,
                             Named("phi") = phivalue));
}

/*
# etsmatrices - function that returns matF and matw.
# Needs to be stand alone to change the damping parameter during the estimation.
# Cvalues includes persistence, phi, initials, intials for seasons, matrixAt coeffs.
*/
// [[Rcpp::export]]
RcppExport SEXP etsmatrices(SEXP matvt, SEXP vecg, SEXP phi, SEXP Cvalues, SEXP ncomponentsR,
                            SEXP lagsModel, SEXP fittertype, SEXP Ttype, SEXP Stype, SEXP nexovars, SEXP matat,
                            SEXP estimpersistence, SEXP estimphi, SEXP estiminit, SEXP estiminitseason, SEXP estimxreg,
                            SEXP matFX, SEXP vecgX, SEXP gowild, SEXP estimFX, SEXP estimgX, SEXP estiminitX){

    NumericMatrix matvt_n(matvt);
    arma::mat matrixVt(matvt_n.begin(), matvt_n.nrow(), matvt_n.ncol());

    NumericMatrix vecg_n(vecg);
    arma::vec vecG(vecg_n.begin(), vecg_n.nrow(), false);

    double phivalue = as<double>(phi);

    NumericMatrix Cv(Cvalues);
    arma::rowvec C(Cv.begin(), Cv.ncol(), false);
    // This is needed for rounded cases, when variance needs to be optimised as well.
    int CLength = Cv.ncol();
    double errorSD = 0.0;

    int ncomponents = as<int>(ncomponentsR);

    IntegerVector lagsModel_n(lagsModel);
    arma::uvec lags = as<arma::uvec>(lagsModel_n);
    int lagsModelMax = max(lags);

    char fitterType = as<char>(fittertype);

    char T = as<char>(Ttype);
    char S = as<char>(Stype);

    int nexo = as<int>(nexovars);

    NumericMatrix matat_n(matat);
    arma::mat matrixAt(matat_n.begin(), matat_n.nrow(), matat_n.ncol());

    bool estimatepersistence = as<bool>(estimpersistence);
    bool estimatephi = as<bool>(estimphi);
    bool estimateinitial = as<bool>(estiminit);
    bool estimateinitialseason = as<bool>(estiminitseason);
    bool estimatexreg = as<bool>(estimxreg);

    NumericMatrix matFX_n(matFX);
    arma::mat matrixFX(matFX_n.begin(), matFX_n.nrow(), matFX_n.ncol());

    NumericMatrix vecgX_n(vecgX);
    arma::vec vecGX(vecgX_n.begin(), vecgX_n.nrow());

    bool wild = as<bool>(gowild);
    bool estimateFX = as<bool>(estimFX);
    bool estimategX = as<bool>(estimgX);
    bool estimateinitialX = as<bool>(estiminitX);

    arma::mat matrixF(1,1,arma::fill::ones);
    arma::mat rowvecW(1,1,arma::fill::ones);

    int currentelement = 0;

    if(estimatepersistence==TRUE){
        vecG = C.cols(currentelement,currentelement + ncomponents-1).t();
        currentelement = currentelement + ncomponents;
    }

    if(estimatephi==TRUE){
        phivalue = as_scalar(C.col(currentelement));
        currentelement = currentelement + 1;
    }

    if((fitterType=='o') | (fitterType=='p')){
        if(estimateinitial==TRUE){
            matrixVt.col(0).fill(as_scalar(C.col(currentelement).t()));
            currentelement = currentelement + 1;
            if(T!='N'){
                matrixVt.col(1).fill(as_scalar(C.col(currentelement).t()));
                currentelement = currentelement + 1;
            }
        }

        if(S!='N'){
            if(estimateinitialseason==TRUE){
                matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1) = C.cols(currentelement, currentelement + lagsModelMax - 1).t();
                currentelement = currentelement + lagsModelMax;
/* # Normalise the initial seasons */
                if(S=='A'){
                    matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1) = matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1) -
                                as_scalar(mean(matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1)));
                }
                else{
                    matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1) = exp(log(matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1)) -
                                as_scalar(mean(log(matrixVt.submat(0,ncomponents-1,lagsModelMax-1,ncomponents-1)))));
                }
            }
        }
    }

    if(estimatexreg==TRUE){
        if(estimateinitialX==TRUE){
            matrixAt.each_row() = C.cols(currentelement,currentelement + nexo - 1);
            currentelement = currentelement + nexo;
        }

        if(wild==TRUE){
            if(estimateFX==TRUE){
                for(int i=0; i < nexo; i = i+1){
                    matrixFX.row(i) = C.cols(currentelement, currentelement + nexo - 1);
                    currentelement = currentelement + nexo;
                }
            }

            if(estimategX==TRUE){
                vecGX = C.cols(currentelement, currentelement + nexo - 1).t();
                currentelement = currentelement + nexo;
            }
        }
    }

    if(currentelement!=CLength){
        errorSD = C(currentelement);
    }

/* # The default values of matrices are set for ZNN  models */
    switch(S){
    case 'N':
        switch(T){
        case 'A':
        case 'M':
            matrixF.set_size(2,2);
            matrixF(0,0) = 1.0;
            matrixF(1,0) = 0.0;
            matrixF(0,1) = phivalue;
            matrixF(1,1) = phivalue;

            rowvecW.set_size(1,2);
            rowvecW(0,0) = 1.0;
            rowvecW(0,1) = phivalue;
        break;
        }
    break;
    case 'A':
    case 'M':
        switch(T){
        case 'A':
        case 'M':
            matrixF.set_size(3,3);
            matrixF.fill(0.0);
            matrixF(0,0) = 1.0;
            matrixF(1,0) = 0.0;
            matrixF(0,1) = phivalue;
            matrixF(1,1) = phivalue;
            matrixF(2,2) = 1.0;

            rowvecW.set_size(1,3);
            rowvecW(0,0) = 1.0;
            rowvecW(0,1) = phivalue;
            rowvecW(0,2) = 1.0;
        break;
        case 'N':
            matrixF.set_size(2,2);
            matrixF.fill(0.0);
            matrixF.diag().fill(1.0);

            rowvecW.set_size(1,2);
            rowvecW.fill(1.0);
        break;
        }
    }

    return wrap(List::create(Named("matF") = matrixF, Named("matw") = rowvecW, Named("vecg") = vecG,
                             Named("phi") = phivalue, Named("matvt") = matrixVt, Named("matat") = matrixAt,
                             Named("matFX") = matrixFX, Named("vecgX") = vecGX, Named("errorSD") = errorSD));
}

// # Fitter for univariate models
List fitter(arma::mat &matrixVt, arma::mat const &matrixF, arma::rowvec const &rowvecW, arma::vec const &vecYt, arma::vec const &vecG,
            arma::uvec &lags, char const &E, char const &T, char const &S,
            arma::mat const &matrixXt, arma::mat &matrixAt, arma::mat const &matrixFX, arma::vec const &vecGX, arma::vec const &vecOt){
    /* # matrixVt should have a length of obs + lagsModelMax.
    * # rowvecW should have 1 row.
    * # vecG should be a vector
    * # lags is a vector of lags
    * # matrixXt is the matrix with the exogenous variables
    * # matrixAt is the matrix with the parameters for the exogenous
    */

    matrixVt = matrixVt.t();
    matrixAt = matrixAt.t();
    arma::mat matrixXtTrans = matrixXt.t();

    int obs = vecYt.n_rows;
    int obsall = matrixVt.n_cols;
    unsigned int nComponents = matrixVt.n_rows;
    arma::uvec lagsInternal = lags;
    unsigned int lagsModelMax = max(lagsInternal);
    int lagslength = lagsInternal.n_rows;

    lagsInternal = lagsInternal * nComponents;

    for(int i=0; i<lagslength; i=i+1){
        lagsInternal(i) = lagsInternal(i) + (lagslength - i - 1);
    }

    arma::uvec lagrows(lagslength, arma::fill::zeros);

    arma::vec vecYfit(obs, arma::fill::zeros);
    arma::vec vecErrors(obs, arma::fill::zeros);
    arma::vec bufferforat(vecGX.n_rows);

    for (unsigned int i=lagsModelMax; i<obs+lagsModelMax; i=i+1) {
        lagrows = i * nComponents - lagsInternal + nComponents - 1;

/* # Measurement equation and the error term */
        // vecOt(i-lagsModelMax) *
        vecYfit(i-lagsModelMax) = wvalue(matrixVt(lagrows), rowvecW, E, T, S,
                                                     matrixXt.row(i-lagsModelMax), matrixAt.col(i-1));

        // This is a failsafe for cases of ridiculously high and ridiculously low values
        if(vecYfit(i-lagsModelMax) > 1e+100){
            vecYfit(i-lagsModelMax) = vecYfit(i-lagsModelMax-1);
        }

        // If this is zero (intermittent), then set error to zero
        if(vecOt(i-lagsModelMax)==0){
            vecErrors(i-lagsModelMax) = 0;
        }
        else{
            vecErrors(i-lagsModelMax) = errorf(vecYt(i-lagsModelMax), vecYfit(i-lagsModelMax), E);
        }

/* # Transition equation */
        matrixVt.col(i) = fvalue(matrixVt(lagrows), matrixF, T, S) +
                          gvalue(matrixVt(lagrows), matrixF, rowvecW, E, T, S) % vecG * vecErrors(i-lagsModelMax);

/* Failsafe for cases when unreasonable value for state vector was produced */
        if(!matrixVt.col(i).is_finite()){
            matrixVt.col(i) = matrixVt(lagrows);
        }
        if((S=='M') & (matrixVt(matrixVt.n_rows-1,i) <= 0)){
            matrixVt(matrixVt.n_rows-1,i) = arma::as_scalar(matrixVt(lagrows.row(matrixVt.n_rows-1)));
        }
        if(T=='M'){
            if((matrixVt(0,i) <= 0) | (matrixVt(1,i) <= 0)){
                matrixVt(0,i) = arma::as_scalar(matrixVt(lagrows.row(0)));
                matrixVt(1,i) = arma::as_scalar(matrixVt(lagrows.row(1)));
            }
        }
        if(any(matrixVt.col(i)>1e+100)){
            matrixVt.col(i) = matrixVt(lagrows);
        }

        if(E=='D'){
            // This is a restriction of values for logistic additive error model
            matrixVt.col(i) = clamp(matrixVt.col(i), -500,500);
        }

/* Renormalise components if the seasonal model is chosen */
        if(S!='N'){
            if(double(i+1) / double(lagsModelMax) == double((i+1) / lagsModelMax)){
                matrixVt.cols(i-lagsModelMax+1,i) = normaliser(matrixVt.cols(i-lagsModelMax+1,i), obsall, lagsModelMax, S, T);
            }
        }

/* # Transition equation for xreg */
        bufferforat = gXvalue(matrixXtTrans.col(i-lagsModelMax), vecGX, vecErrors.row(i-lagsModelMax), E);
        matrixAt.col(i) = matrixFX * matrixAt.col(i-1) + bufferforat;
    }

    for (int i=obs+lagsModelMax; i<obsall; i=i+1) {
        lagrows = i * nComponents - lagsInternal + nComponents - 1;
        matrixVt.col(i) = fvalue(matrixVt(lagrows), matrixF, T, S);
        matrixAt.col(i) = matrixFX * matrixAt.col(i-1);

/* Failsafe for cases when unreasonable value for state vector was produced */
        if(!matrixVt.col(i).is_finite()){
            matrixVt.col(i) = matrixVt(lagrows);
        }
        if((S=='M') & (matrixVt(matrixVt.n_rows-1,i) <= 0)){
            matrixVt(matrixVt.n_rows-1,i) = arma::as_scalar(matrixVt(lagrows.row(matrixVt.n_rows-1)));
        }
        if(T=='M'){
            if((matrixVt(0,i) <= 0) | (matrixVt(1,i) <= 0)){
                matrixVt(0,i) = arma::as_scalar(matrixVt(lagrows.row(0)));
                matrixVt(1,i) = arma::as_scalar(matrixVt(lagrows.row(1)));
            }
        }

        if(E=='D'){
            // This is a restriction of values for logistic additive error model
            matrixVt.col(i) = clamp(matrixVt.col(i), -500,500);
        }
    }

    if(E=='D'){
        // This is a logistic additive error
        vecYfit = clamp(vecYfit, -500,500);
        vecYfit = exp(vecYfit) / (1 + exp(vecYfit));
    }
    else if(E=='L'){
        // This is a logistic multiplicative error
        vecYfit = vecYfit / (1 + vecYfit);
    }

    return List::create(Named("matvt") = matrixVt.t(), Named("yfit") = vecYfit,
                        Named("errors") = vecErrors, Named("matat") = matrixAt.t());
}

// # Backfitter for univariate models
List backfitter(arma::mat &matrixVt, arma::mat const &matrixF, arma::rowvec const &rowvecW, arma::vec const &vecYt, arma::vec const &vecG,
                arma::uvec &lags, char const &E, char const &T, char const &S,
                arma::mat const &matrixXt, arma::mat &matrixAt, arma::mat const &matrixFX, arma::vec const &vecGX, arma::vec const &vecOt){
    /* # matrixVt should have a length of obs + lagsModelMax.
    * # rowvecW should have 1 row.
    * # matgt should be a vector
    * # lags is a vector of lags
    * # matrixXt is the matrix with the exogenous variables
    * # matrixAt is the matrix with the parameters for the exogenous
    */

    int nloops;
    // If... this is ssarima, don't do backcasting
    if(all(lags==1) & (lags.n_elem > 3)){
        nloops = 0;
    }
    else{
    // This means that we do only one loop (forward -> backwards -> forward)
        nloops = 1;
    }

    matrixVt = matrixVt.t();
    matrixAt = matrixAt.t();
    arma::mat matrixXtTrans = matrixXt.t();

    // Inverse transition matrix for backcasting
    // arma::mat matrixFInv = matrixF;
    // arma::rowvec rowvecWInv = rowvecW;
    // Values needed for eigenvalues calculation
    // arma::cx_vec eigval;
    //
    // if(arma::eig_gen(eigval, matrixF)){
    //     if(any(abs(eigval) >= 1)){
    //         if(!arma::inv(matrixFInv,matrixF)){
    //             matrixFInv = arma::pinv(matrixF);
    //             rowvecWInv = rowvecW * matrixFInv * matrixFInv;
    //         }
    //     }
    // }

    // if(!arma::inv(matrixFInv,matrixF)){
    //     matrixFInv = matrixF.t();
    //     matrixFInv(0,0) = 0;
    //     matrixFInv.col(matrixFInv.n_cols-1) = flipud(matrixF.col(0));
    //     // matrixFInv.col(matrixFInv.n_cols-1) = matrixF.col(0);
    // }
    // // arma::vec vecGInv = vecG - matrixF.col(0) + matrixFInv.col(0);
    // arma::rowvec rowvecWInv = rowvecW * matrixFInv * matrixFInv;

    int obs = vecYt.n_rows;
    int obsall = matrixVt.n_cols;
    unsigned int nComponents = matrixVt.n_rows;
    arma::uvec lagsModifier = lags;
    arma::uvec lagsInternal = lags;
    unsigned int lagsModelMax = max(lagsInternal);
    int lagslength = lagsInternal.n_rows;

    lagsInternal = lagsInternal * nComponents;

    for(int i=0; i<lagslength; i=i+1){
        lagsModifier(i) = lagslength - i - 1;
    }

    arma::uvec lagrows(lagslength, arma::fill::zeros);

    arma::vec vecYfit(obs, arma::fill::zeros);
    arma::vec vecErrors(obs, arma::fill::zeros);
    arma::vec bufferforat(vecGX.n_rows);

    for(int j=0; j<=nloops; j=j+1){
/* ### Go forward ### */
        for (unsigned int i=lagsModelMax; i<obs+lagsModelMax; i=i+1) {
            lagrows = i * nComponents - (lagsInternal + lagsModifier) + nComponents - 1;

/* # Measurement equation and the error term */
            // vecOt(i-lagsModelMax) *
            vecYfit(i-lagsModelMax) = wvalue(matrixVt(lagrows), rowvecW, E, T, S,
                                                         matrixXt.row(i-lagsModelMax), matrixAt.col(i-1));

            // This is a failsafe for cases of ridiculously high and ridiculously low values
            if(vecYfit(i-lagsModelMax) > 1e+100){
                vecYfit(i-lagsModelMax) = vecYfit(i-lagsModelMax-1);
            }

            // If this is zero (intermittent), then set error to zero
            if(vecOt(i-lagsModelMax)==0){
                vecErrors(i-lagsModelMax) = 0;
            }
            else{
                vecErrors(i-lagsModelMax) = errorf(vecYt(i-lagsModelMax), vecYfit(i-lagsModelMax), E);
            }

            // This is a failsafe for cases of ridiculously high and ridiculously low values
            if(!vecYfit.row(i-lagsModelMax).is_finite()){
                vecYfit(i-lagsModelMax) = vecYfit(i-lagsModelMax-1);
            }

/* # Transition equation */
            matrixVt.col(i) = fvalue(matrixVt(lagrows), matrixF, T, S) +
                              gvalue(matrixVt(lagrows), matrixF, rowvecW, E, T, S) % vecG * vecErrors(i-lagsModelMax);

/* Failsafe for cases when unreasonable value for state vector was produced */
            if(!matrixVt.col(i).is_finite()){
                matrixVt.col(i) = matrixVt(lagrows);
            }
            if((S=='M') & (matrixVt(matrixVt.n_rows-1,i) <= 0)){
                matrixVt(matrixVt.n_rows-1,i) = arma::as_scalar(matrixVt(lagrows.row(matrixVt.n_rows-1)));
            }
            if(T=='M'){
                if((matrixVt(0,i) <= 0) | (matrixVt(1,i) <= 0)){
                    matrixVt(0,i) = arma::as_scalar(matrixVt(lagrows.row(0)));
                    matrixVt(1,i) = arma::as_scalar(matrixVt(lagrows.row(1)));
                }
            }
            if(any(matrixVt.col(i)>1e+100)){
                matrixVt.col(i) = matrixVt(lagrows);
            }

            if(E=='D'){
                // This is a restriction of values for logistic additive error model
                matrixVt.col(i) = clamp(matrixVt.col(i), -500,500);
            }

/* Renormalise components if the seasonal model is chosen */
            if(S!='N'){
                if(double(i+1) / double(lagsModelMax) == double((i+1) / lagsModelMax)){
                    matrixVt.cols(i-lagsModelMax+1,i) = normaliser(matrixVt.cols(i-lagsModelMax+1,i), obsall, lagsModelMax, S, T);
                }
            }

/* # Transition equation for xreg */
            bufferforat = gXvalue(matrixXtTrans.col(i-lagsModelMax), vecGX, vecErrors.row(i-lagsModelMax), E);
            matrixAt.col(i) = matrixFX * matrixAt.col(i-1) + bufferforat;
        }

        for (int i=obs+lagsModelMax; i<obsall; i=i+1) {
            lagrows = i * nComponents - (lagsInternal + lagsModifier) + nComponents - 1;
            matrixVt.col(i) = fvalue(matrixVt(lagrows), matrixF, T, S);
            matrixAt.col(i) = matrixFX * matrixAt.col(i-1);

/* Failsafe for cases when unreasonable value for state vector was produced */
            if(!matrixVt.col(i).is_finite()){
                matrixVt.col(i) = matrixVt(lagrows);
            }
            if((S=='M') & (matrixVt(matrixVt.n_rows-1,i) <= 0)){
                matrixVt(matrixVt.n_rows-1,i) = arma::as_scalar(matrixVt(lagrows.row(matrixVt.n_rows-1)));
            }
            if(T=='M'){
                if((matrixVt(0,i) <= 0) | (matrixVt(1,i) <= 0)){
                    matrixVt(0,i) = arma::as_scalar(matrixVt(lagrows.row(0)));
                    matrixVt(1,i) = arma::as_scalar(matrixVt(lagrows.row(1)));
                }
            }
            if(any(matrixVt.col(i)>1e+100)){
                matrixVt.col(i) = matrixVt(lagrows);
            }

            if(E=='D'){
                // This is a restriction of values for logistic additive error model
                matrixVt.col(i) = clamp(matrixVt.col(i), -500,500);
            }
        }

/* # If this is the last loop, stop here and don't do backcast */
        if(j==nloops){
            break;
        }

/* ### Now go back ### */
        for (unsigned int i=obs+lagsModelMax-1; i>=lagsModelMax; i=i-1) {
            lagrows = i * nComponents + lagsInternal - lagsModifier + nComponents - 1;

/* # Measurement equation and the error term */
            // vecOt(i-lagsModelMax) *
            vecYfit.row(i-lagsModelMax) = wvalue(matrixVt(lagrows), rowvecW, E, T, S,
                                                             matrixXt.row(i-lagsModelMax), matrixAt.col(i+1));

            // This is for cases of ridiculously high and ridiculously low values
            if(vecYfit(i-lagsModelMax) > 1e+100){
                vecYfit(i-lagsModelMax) = vecYfit(i-lagsModelMax+1);
            }

            // If this is zero (intermittent), then set error to zero
            if(vecOt(i-lagsModelMax)==0){
                vecErrors(i-lagsModelMax) = 0;
            }
            else{
                vecErrors(i-lagsModelMax) = errorf(vecYt(i-lagsModelMax), vecYfit(i-lagsModelMax), E);
            }

/* # Transition equation */
            matrixVt.col(i) = fvalue(matrixVt(lagrows), matrixF, T, S) +
                              gvalue(matrixVt(lagrows), matrixF, rowvecW, E, T, S) % vecG * vecErrors(i-lagsModelMax);

/* Failsafe for cases when unreasonable value for state vector was produced */
            if(!matrixVt.col(i).is_finite()){
                matrixVt.col(i) = matrixVt(lagrows);
            }
            if((S=='M') & (matrixVt(matrixVt.n_rows-1,i) <= 0)){
                matrixVt(matrixVt.n_rows-1,i) = arma::as_scalar(matrixVt(lagrows.row(matrixVt.n_rows-1)));
            }
            if(T=='M'){
                if((matrixVt(0,i) <= 0) | (matrixVt(1,i) <= 0)){
                    matrixVt(0,i) = arma::as_scalar(matrixVt(lagrows.row(0)));
                    matrixVt(1,i) = arma::as_scalar(matrixVt(lagrows.row(1)));
                }
            }
            if(any(matrixVt.col(i)>1e+100)){
                matrixVt.col(i) = matrixVt(lagrows);
            }

            if(E=='D'){
                // This is a restriction of values for logistic additive error model
                matrixVt.col(i) = clamp(matrixVt.col(i), -500,500);
            }

/* Skipping renormalisation of components in backcasting */

/* # Transition equation for xreg */
            bufferforat = gXvalue(matrixXtTrans.col(i-lagsModelMax), vecGX, vecErrors.row(i-lagsModelMax), E);
            matrixAt.col(i) = matrixFX * matrixAt.col(i+1) + bufferforat;
        }
/* # Fill in the head of the matrices */
        for (int i=lagsModelMax-1; i>=0; i=i-1) {
            lagrows = i * nComponents + lagsInternal - lagsModifier + nComponents - 1;
            matrixVt.col(i) = fvalue(matrixVt(lagrows), matrixF, T, S);
            matrixAt.col(i) = matrixFX * matrixAt.col(i+1);

/* Failsafe for cases when unreasonable value for state vector was produced */
            if(!matrixVt.col(i).is_finite()){
                matrixVt.col(i) = matrixVt(lagrows);
            }
            if((S=='M') & (matrixVt(matrixVt.n_rows-1,i) <= 0)){
                matrixVt(matrixVt.n_rows-1,i) = arma::as_scalar(matrixVt(lagrows.row(matrixVt.n_rows-1)));
            }
            if(T=='M'){
                if((matrixVt(0,i) <= 0) | (matrixVt(1,i) <= 0)){
                    matrixVt(0,i) = arma::as_scalar(matrixVt(lagrows.row(0)));
                    matrixVt(1,i) = arma::as_scalar(matrixVt(lagrows.row(1)));
                }
            }
            if(any(matrixVt.col(i)>1e+100)){
                matrixVt.col(i) = matrixVt(lagrows);
            }

            if(E=='D'){
                // This is a restriction of values for logistic additive error model
                matrixVt.col(i) = clamp(matrixVt.col(i), -500,500);
            }
        }
    }

    if(E=='D'){
        // This is a logistic additive error
        vecYfit = clamp(vecYfit, -500,500);
        vecYfit = exp(vecYfit) / (1 + exp(vecYfit));
    }
    else if(E=='L'){
        // This is a logistic multiplicative error
        vecYfit = vecYfit / (1 + vecYfit);
    }

    return List::create(Named("matvt") = matrixVt.t(), Named("yfit") = vecYfit,
                        Named("errors") = vecErrors, Named("matat") = matrixAt.t());
}

/* # Wrapper for fitter */
// [[Rcpp::export]]
RcppExport SEXP fitterwrap(SEXP matvt, SEXP matF, SEXP matw, SEXP yt, SEXP vecg,
                            SEXP lagsModel, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP fittertype,
                            SEXP matxt, SEXP matat, SEXP matFX, SEXP vecgX, SEXP ot) {

    NumericMatrix matvt_n(matvt);
    arma::mat matrixVt(matvt_n.begin(), matvt_n.nrow(), matvt_n.ncol());

    NumericMatrix matF_n(matF);
    arma::mat matrixF(matF_n.begin(), matF_n.nrow(), matF_n.ncol(), false);

    NumericMatrix matw_n(matw);
    arma::rowvec rowvecW(matw_n.begin(), matw_n.ncol(), false);

    NumericMatrix yt_n(yt);
    arma::vec vecYt(yt_n.begin(), yt_n.nrow(), false);

    NumericMatrix vecg_n(vecg);
    arma::vec vecG(vecg_n.begin(), vecg_n.nrow(), false);

    IntegerVector lagsModel_n(lagsModel);
    arma::uvec lags = as<arma::uvec>(lagsModel_n);

    char E = as<char>(Etype);
    char T = as<char>(Ttype);
    char S = as<char>(Stype);

    char fitterType = as<char>(fittertype);

    NumericMatrix matxt_n(matxt);
    arma::mat matrixXt(matxt_n.begin(), matxt_n.nrow(), matxt_n.ncol(), false);

    NumericMatrix matat_n(matat);
    arma::mat matrixAt(matat_n.begin(), matat_n.nrow(), matat_n.ncol());

    NumericMatrix matFX_n(matFX);
    arma::mat matrixFX(matFX_n.begin(), matFX_n.nrow(), matFX_n.ncol(), false);

    NumericMatrix vecgX_n(vecgX);
    arma::vec vecGX(vecgX_n.begin(), vecgX_n.nrow(), false);

    NumericVector ot_n(ot);
    arma::vec vecOt(ot_n.begin(), ot_n.size(), false);

    switch(fitterType){
        case 'b':
            return wrap(backfitter(matrixVt, matrixF, rowvecW, vecYt, vecG, lags, E, T, S,
                                   matrixXt, matrixAt, matrixFX, vecGX, vecOt));
        break;
        case 'o':
        default:
            return wrap(fitter(matrixVt, matrixF, rowvecW, vecYt, vecG, lags, E, T, S,
                               matrixXt, matrixAt, matrixFX, vecGX, vecOt));
    }
}

/* # Function produces the point forecasts for the specified model */
arma::mat forecaster(arma::mat matrixVt, arma::mat const &matrixF, arma::rowvec const &rowvecW,
                     unsigned int const &hor, char const &E, char const &T, char const &S, arma::uvec lags,
                     arma::mat matrixXt, arma::mat matrixAt, arma::mat const &matrixFX){
    int lagslength = lags.n_rows;
    unsigned int lagsModelMax = max(lags);
    unsigned int hh = hor + lagsModelMax;

    arma::uvec lagrows(lagslength, arma::fill::zeros);
    arma::vec matyfor(hor, arma::fill::zeros);
    arma::mat matrixVtnew(hh, matrixVt.n_cols, arma::fill::zeros);
    arma::mat matrixAtnew(hh, matrixAt.n_cols, arma::fill::zeros);

    lags = lagsModelMax - lags;
    for(int i=1; i<lagslength; i=i+1){
        lags(i) = lags(i) + hh * i;
    }

    matrixVtnew.submat(0,0,lagsModelMax-1,matrixVtnew.n_cols-1) = matrixVt.submat(0,0,lagsModelMax-1,matrixVtnew.n_cols-1);
    matrixAtnew.submat(0,0,lagsModelMax-1,matrixAtnew.n_cols-1) = matrixAtnew.submat(0,0,lagsModelMax-1,matrixAtnew.n_cols-1);

/* # Fill in the new xt matrix using F. Do the forecasts. */
    for (unsigned int i=lagsModelMax; i<(hor+lagsModelMax); i=i+1) {
        lagrows = lags - lagsModelMax + i;
        matrixVtnew.row(i) = arma::trans(fvalue(matrixVtnew(lagrows), matrixF, T, S));
        matrixAtnew.row(i) = matrixAtnew.row(i-1) * matrixFX;

        matyfor.row(i-lagsModelMax) = (wvalue(matrixVtnew(lagrows), rowvecW, E, T, S, matrixXt.row(i-lagsModelMax), trans(matrixAt.row(i-lagsModelMax))));
    }

    return matyfor;
}

/* # Wrapper for forecaster */
// [[Rcpp::export]]
RcppExport SEXP forecasterwrap(SEXP matvt, SEXP matF, SEXP matw,
                               SEXP h, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP lagsModel,
                               SEXP matxt, SEXP matat, SEXP matFX){
    NumericMatrix matvt_n(matvt);
    arma::mat matrixVt(matvt_n.begin(), matvt_n.nrow(), matvt_n.ncol(), false);

    NumericMatrix matF_n(matF);
    arma::mat matrixF(matF_n.begin(), matF_n.nrow(), matF_n.ncol(), false);

    NumericMatrix matw_n(matw);
    arma::mat rowvecW(matw_n.begin(), matw_n.nrow(), matw_n.ncol(), false);

    unsigned int hor = as<int>(h);
    char E = as<char>(Etype);
    char T = as<char>(Ttype);
    char S = as<char>(Stype);

    IntegerVector lagsModel_n(lagsModel);
    arma::uvec lags = as<arma::uvec>(lagsModel_n);

    NumericMatrix matxt_n(matxt);
    arma::mat matrixXt(matxt_n.begin(), matxt_n.nrow(), matxt_n.ncol(), false);

    NumericMatrix matat_n(matat);
    arma::mat matrixAt(matat_n.begin(), matat_n.nrow(), matat_n.ncol());

    NumericMatrix matFX_n(matFX);
    arma::mat matrixFX(matFX_n.begin(), matFX_n.nrow(), matFX_n.ncol(), false);

    return wrap(forecaster(matrixVt, matrixF, rowvecW, hor, E, T, S, lags, matrixXt, matrixAt, matrixFX));
}

/* # Function produces matrix of errors based on multisteps forecast */
arma::mat errorer(arma::mat const &matrixVt, arma::mat const &matrixF, arma::mat const &rowvecW, arma::mat const &vecYt,
                  int const &hor, char const &E, char const &T, char const &S, arma::uvec const &lags,
                  arma::mat const &matrixXt, arma::mat const &matrixAt, arma::mat const &matrixFX, arma::vec const &vecOt){
    int obs = vecYt.n_rows;
    // This is needed for cases, when hor>obs
    int hh = 0;
    arma::mat matErrors(obs, hor, arma::fill::zeros);
    unsigned int lagsModelMax = max(lags);

    for(int i = 0; i < (obs-hor); i=i+1){
        hh = std::min(hor, obs-i);
        matErrors.submat(i, 0, i, hh-1) = arma::trans(vecOt.rows(i, i+hh-1) % errorvf(vecYt.rows(i, i+hh-1),
            forecaster(matrixVt.rows(i,i+lagsModelMax-1), matrixF, rowvecW, hh, E, T, S, lags, matrixXt.rows(i, i+hh-1),
                matrixAt.rows(i, i+hh-1), matrixFX), E));
    }

    // Cut-off the redundant last part
    if(obs>hor){
        matErrors = matErrors.rows(0,obs-hor-1);
    }

// Fix for GV in order to perform better in the sides of the series
    // for(int i=0; i<(hor-1); i=i+1){
    //     matErrors.submat((hor-2)-(i),i+1,(hor-2)-(i),hor-1) = matErrors.submat(hor-1,0,hor-1,hor-i-2) * sqrt(1.0+i);
    // }

    return matErrors;
}

/* # Wrapper for errorer */
// [[Rcpp::export]]
RcppExport SEXP errorerwrap(SEXP matvt, SEXP matF, SEXP matw, SEXP yt,
                            SEXP h, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP lagsModel,
                            SEXP matxt, SEXP matat, SEXP matFX, SEXP ot){
    NumericMatrix matvt_n(matvt);
    arma::mat matrixVt(matvt_n.begin(), matvt_n.nrow(), matvt_n.ncol(), false);

    NumericMatrix matF_n(matF);
    arma::mat matrixF(matF_n.begin(), matF_n.nrow(), matF_n.ncol(), false);

    NumericMatrix matw_n(matw);
    arma::rowvec rowvecW(matw_n.begin(), matw_n.ncol(), false);

    NumericMatrix yt_n(yt);
    arma::vec vecYt(yt_n.begin(), yt_n.nrow(), false);

    int hor = as<int>(h);
    char E = as<char>(Etype);
    char T = as<char>(Ttype);
    char S = as<char>(Stype);

    IntegerVector lagsModel_n(lagsModel);
    arma::uvec lags = as<arma::uvec>(lagsModel_n);

    NumericMatrix matxt_n(matxt);
    arma::mat matrixXt(matxt_n.begin(), matxt_n.nrow(), matxt_n.ncol(), false);

    NumericMatrix matat_n(matat);
    arma::mat matrixAt(matat_n.begin(), matat_n.nrow(), matat_n.ncol());

    NumericMatrix matFX_n(matFX);
    arma::mat matrixFX(matFX_n.begin(), matFX_n.nrow(), matFX_n.ncol(), false);

    NumericVector ot_n(ot);
    arma::vec vecOt(ot_n.begin(), ot_n.size(), false);

    return wrap(errorer(matrixVt, matrixF, rowvecW, vecYt,
                        hor, E, T, S, lags,
                        matrixXt, matrixAt, matrixFX, vecOt));
}

int CFtypeswitch (std::string const& CFtype) {
    // MSE, MAE, HAM, MSEh, TMSE, GTMSE, aMSEh, aTMSE, aGTMSE, MAEh, TMAE, GTMAE, HAMh, THAM, GTHAM,
    // GPL, aGPL, Rounded, TSB
    if(CFtype=="MSE") return 1;
    else if(CFtype=="MAE") return 2;
    else if(CFtype=="HAM") return 3;
    else if(CFtype=="MSEh") return 4;
    else if(CFtype=="TMSE") return 5;
    else if(CFtype=="GTMSE") return 6;
    else if(CFtype=="MSCE") return 7;
    else if(CFtype=="MAEh") return 8;
    else if(CFtype=="TMAE") return 9;
    else if(CFtype=="GTMAE") return 10;
    else if(CFtype=="MACE") return 11;
    else if(CFtype=="HAMh") return 12;
    else if(CFtype=="THAM") return 13;
    else if(CFtype=="GTHAM") return 14;
    else if(CFtype=="CHAM") return 15;
    else if(CFtype=="GPL") return 16;
    else if(CFtype=="aMSEh") return 17;
    else if(CFtype=="aTMSE") return 18;
    else if(CFtype=="aGTMSE") return 19;
    else if(CFtype=="aGPL") return 20;
    // else if(CFtype=="aMSCE") return 18;
    else if(CFtype=="Rounded") return 21;
    else if(CFtype=="TSB") return 22;
    else return 1;
}

/* # Function returns the chosen Cost Function based on the chosen model and produced errors */
double optimizer(arma::mat &matrixVt, arma::mat const &matrixF, arma::rowvec const &rowvecW, arma::vec const &vecYt, arma::vec const &vecG,
                 unsigned int const &hor, arma::uvec &lags, char const &E, char const &T, char const &S,
                 bool const &multi, std::string const &CFtype, double const &normalize, char const &fitterType,
                 arma::mat const &matrixXt, arma::mat &matrixAt, arma::mat const &matrixFX, arma::vec const &vecGX, arma::vec const &vecOt,
                 double const &errorSD){
// # Make decomposition functions shut up!
    std::ostream nullstream(0);
    arma::set_cerr_stream(nullstream);

    // These lines of code are needed for logistic probability:
    // We define E as 'L' or 'D' in this case and use TSB CF.
    char ENew = E;
    std::string CFtypeNew = CFtype;
    if(CFtype=="LogisticD"){
        ENew = 'D';
        CFtypeNew = "TSB";
    }
    else if(CFtype=="LogisticL"){
        ENew = 'L';
        CFtypeNew = "TSB";
    }

    int CFSwitch = CFtypeswitch(CFtypeNew);

    arma::uvec nonzeroes = find(vecOt>0);
    unsigned int obs = vecYt.n_rows;
    double CFres = 0;
    int matobs = obs - hor;

// yactsum is needed for multiplicative error models
    double yactsum = 0;
    if((CFSwitch==7) | (CFSwitch==11) | (CFSwitch==15)){
        arma::vec vecYtNonZero = vecYt.elem(nonzeroes);
        for(unsigned int i=0; i<(obs-hor); ++i){
            yactsum += log(sum(vecYtNonZero.rows(i,i+hor)));
        }
    }
    else{
        yactsum = arma::as_scalar(sum(log(vecYt.elem(nonzeroes))));
    }

    List fitting;

    switch(fitterType){
        case 'b':
        fitting = backfitter(matrixVt, matrixF, rowvecW, vecYt, vecG, lags, ENew, T, S,
                             matrixXt, matrixAt, matrixFX, vecGX, vecOt);
        break;
        case 'o':
        default:
        fitting = fitter(matrixVt, matrixF, rowvecW, vecYt, vecG, lags, ENew, T, S,
                         matrixXt, matrixAt, matrixFX, vecGX, vecOt);
    }

    NumericMatrix mvtfromfit = as<NumericMatrix>(fitting["matvt"]);
    matrixVt = as<arma::mat>(mvtfromfit);
    NumericMatrix errorsfromfit = as<NumericMatrix>(fitting["errors"]);
    NumericMatrix matrixAtfromfit = as<NumericMatrix>(fitting["matat"]);
    matrixAt = as<arma::mat>(matrixAtfromfit);

    arma::mat matErrors;

    arma::vec veccij(hor, arma::fill::ones);
    arma::mat matrixSigma(hor, hor, arma::fill::eye);

    if((multi==true) & (CFSwitch<=16)){
        matErrors = errorer(matrixVt, matrixF, rowvecW, vecYt, hor, E, T, S, lags, matrixXt, matrixAt, matrixFX, vecOt);
        if(E=='M'){
            matErrors = log(1 + matErrors);
            matErrors.elem(arma::find_nonfinite(matErrors)).fill(1e10);

// This correction is needed in order to take the correct number of observations in the error matrix
// !!! This needs to be revised !!! ///
            yactsum = yactsum / obs * matobs;
        }
    }
    else{
        arma::mat matErrorsfromfit(errorsfromfit.begin(), errorsfromfit.nrow(), errorsfromfit.ncol(), false);
        matErrors = matErrorsfromfit;
        matErrors = matErrors.elem(nonzeroes);
        if(E=='M'){
            matErrors = log(1 + matErrors);
        }
    }

// If this is an analytical multistep cost function
    if((CFSwitch>16) & (CFSwitch<21)){
// Form vector for basic values and matrix Mu
        for(unsigned int i=1; i<hor; ++i){
            veccij(i) = arma::as_scalar(rowvecW * matrixPower(matrixF,i) * vecG);
        }

// Fill in the diagonal of Sigma matrix
        for(unsigned int i=1; i<hor; ++i){
            matrixSigma(i,i) = matrixSigma(i-1,i-1) + pow(veccij(i),2);
        }

// aGPL
        if(CFSwitch==20){
            for(unsigned int i=0; i<hor; ++i){
                for(unsigned int j=0; j<hor; ++j){
                    if(i>=j){
                        continue;
                    }
                    if(i==0){
                        matrixSigma(i,j) = veccij(j);
                    }
                    else{
                        matrixSigma(i,j) = veccij(j-i) + sum(veccij.rows(j-i+1,j) % veccij.rows(1,i));
                    }
                }
            }
            matrixSigma = symmatu(matrixSigma);
        }
    }

    // Rounded CF and TSB
    arma::vec vecYfit;
    if(CFSwitch>=21){
        NumericMatrix yfitfromfit = as<NumericMatrix>(fitting["yfit"]);
        vecYfit = as<arma::vec>(yfitfromfit);
    }

    switch(E){
    case 'A':
        switch(CFSwitch){
        // Basic one-step aheads
        case 1:
            CFres = arma::as_scalar(sum(pow(matErrors,2)) / double(obs));
        break;
        case 2:
            CFres = arma::as_scalar(sum(abs(matErrors)) / double(obs));
        break;
        case 3:
            CFres = arma::as_scalar(sum(sqrt(abs(matErrors))) / (2*double(obs)));
        break;
        // MSE based multisteps:
        case 4:
            CFres = arma::as_scalar(sum(pow(matErrors.col(hor-1),2)) / double(matobs));
        break;
        case 5:
            CFres = arma::as_scalar(sum(sum(pow(matErrors,2)) / double(matobs), 1));
        break;
        case 6:
            CFres = arma::as_scalar(sum(log(sum(pow(matErrors,2)) / double(matobs)), 1));
        break;
        case 7:
            CFres = arma::as_scalar(sum(pow(sum(matErrors,1),2) / double(matobs)));
        break;
        // MAE based multisteps:
        case 8:
            CFres = arma::as_scalar(sum(abs(matErrors.col(hor-1))) / double(matobs));
        break;
        case 9:
            CFres = arma::as_scalar(sum(sum(abs(matErrors)) / double(matobs), 1));
        break;
        case 10:
            CFres = arma::as_scalar(sum(log(sum(abs(matErrors)) / double(matobs)), 1));
        break;
        case 11:
            CFres = arma::as_scalar(sum(abs(sum(matErrors,1)) / double(matobs)));
        break;
        // HAM based multisteps:
        case 12:
            CFres = arma::as_scalar(sum(sqrt(abs(matErrors.col(hor-1)))) / double(matobs));
        break;
        case 13:
            CFres = arma::as_scalar(sum(sum(sqrt(abs(matErrors))) / double(matobs), 1));
        break;
        case 14:
            CFres = arma::as_scalar(sum(log(sum(sqrt(abs(matErrors))) / double(matobs)), 1));
        break;
        case 15:
            CFres = arma::as_scalar(sum(sqrt(abs(sum(matErrors,1))) / double(matobs)));
        break;
        // GPL
        case 16:
            try{
                CFres = double(log(arma::prod(eig_sym(trans(matErrors / normalize) * (matErrors / normalize) / matobs))) +
                    hor * log(pow(normalize,2)));
            }
            catch(const std::runtime_error&){
                CFres = double(log(arma::det(arma::trans(matErrors / normalize) * (matErrors / normalize) / matobs)) +
                    hor * log(pow(normalize,2)));
            }
        break;
        // Analytical multisteps
        case 17:
            CFres = (as_scalar(mean(pow(matErrors,2))) * matrixSigma(hor-1,hor-1));
        break;
        case 18:
            CFres = arma::trace(as_scalar(mean(pow(matErrors,2))) * matrixSigma);
        break;
        case 19:
        case 20:
            try{
                CFres = double(log(arma::prod(eig_sym(as_scalar(mean(pow(matErrors / normalize,2))) * matrixSigma
                                                          ))) + hor*log(pow(normalize,2)));
            }
            catch(const std::runtime_error&){
                CFres = log(arma::det(as_scalar(mean(pow(matErrors / normalize,2))) * matrixSigma
                                          )) + hor*log(pow(normalize,2));
            }
        break;
        // Rounded
        case 21:
            CFres = 0;
        break;
        // TSB
        case 22:
            // 0.5 is needed for cases, when the variable is continuous in (0, 1)
            CFres = -sum(log(vecYfit.elem(find(vecYt>=0.5)))) - sum(log(1-vecYfit.elem(find(vecYt<0.5))));
        }
    break;
    case 'M':
    // (2 / double(obs)) and others are needed here in order to produce adequate likelihoods
    // exp(log(...) + ...) is needed in order to have the necessary part before producing logs in likelihood
        switch(CFSwitch){
        // Basic one-step aheads
        case 1:
            CFres = arma::as_scalar(sum(pow(matErrors,2)) / double(obs));
        break;
        case 2:
            CFres = arma::as_scalar(sum(abs(matErrors)) / double(obs));
        break;
        case 3:
            CFres = arma::as_scalar(sum(sqrt(abs(matErrors))) / (2*double(obs)));
        break;
        // MSE based multisteps:
        case 4:
            CFres = arma::as_scalar(sum(pow(matErrors.col(hor-1),2)) / double(matobs));
        break;
        case 5:
            CFres = arma::as_scalar(sum(sum(pow(matErrors,2)) / double(matobs), 1));
        break;
        case 6:
            CFres = arma::as_scalar(sum(log(sum(pow(matErrors,2)) / double(matobs)), 1));
        break;
        case 7:
            CFres = arma::as_scalar(sum(pow(sum(matErrors,1),2) / double(matobs)));
        break;
        // MAE based multisteps:
        case 8:
            CFres = arma::as_scalar(sum(abs(matErrors.col(hor-1))) / double(matobs));
        break;
        case 9:
            CFres = arma::as_scalar(sum(sum(abs(matErrors)) / double(matobs), 1));
        break;
        case 10:
            CFres = arma::as_scalar(sum(log(sum(abs(matErrors)) / double(matobs)), 1));
        break;
        case 11:
            CFres = arma::as_scalar(sum(abs(sum(matErrors,1)) / double(matobs)));
        break;
        // HAM based multisteps:
        case 12:
            CFres = arma::as_scalar(sum(sqrt(abs(matErrors.col(hor-1)))) / double(matobs));
        break;
        case 13:
            CFres = arma::as_scalar(sum(sum(sqrt(abs(matErrors))) / double(matobs), 1));
        break;
        case 14:
            CFres = arma::as_scalar(sum(log(sum(sqrt(abs(matErrors))) / double(matobs)), 1));
        break;
        case 15:
            CFres = arma::as_scalar(sum(sqrt(abs(sum(matErrors,1))) / double(matobs)));
        break;
        // GPL
        case 16:
            try{
                CFres = double(log(arma::prod(eig_sym(trans(matErrors) * (matErrors) / matobs))));
            }
            catch(const std::runtime_error&){
                CFres = double(log(arma::det(arma::trans(matErrors) * matErrors / double(matobs))));
            }
            // CFres = CFres + (2 / double(matobs)) * double(hor) * yactsum;
        break;
        // Analytical multisteps
        case 17:
            CFres = (as_scalar(mean(pow(matErrors,2))) * matrixSigma(hor-1,hor-1));
            // CFres = CFres + (2 / double(matobs)) * yactsum;
        break;
        case 18:
            CFres = arma::trace(as_scalar(mean(pow(matErrors,2))) * matrixSigma);
            // CFres = CFres + (2 / double(matobs)) * double(hor) * yactsum;
        break;
        case 19:
        case 20:
            try{
                CFres = double(log(arma::prod(eig_sym(as_scalar(mean(pow(matErrors / normalize,2))) * matrixSigma
                                                          ))) + hor*log(pow(normalize,2)));
            }
            catch(const std::runtime_error&){
                CFres = log(arma::det(as_scalar(mean(pow(matErrors / normalize,2))) * matrixSigma
                                          )) + hor*log(pow(normalize,2));
            }
            // CFres = CFres + (2 / double(matobs)) * double(hor) * yactsum;
        break;
        // Rounded
        case 21:
            CFres = 0;
        break;
        // TSB
        case 22:
            // 0.5 is needed for cases, when the variable is continuous in (0, 1)
            CFres = -sum(log(vecYfit.elem(find(vecYt>=0.5)))) - sum(log(1-vecYfit.elem(find(vecYt<0.5))));
        }
    }
    return CFres;
}

/* # Wrapper for optimiser */
// [[Rcpp::export]]
RcppExport SEXP optimizerwrap(SEXP matvt, SEXP matF, SEXP matw, SEXP yt, SEXP vecg,
                              SEXP h, SEXP lagsModel, SEXP Etype, SEXP Ttype, SEXP Stype,
                              SEXP multisteps, SEXP CFt, SEXP normalizer, SEXP fittertype,
                              SEXP matxt, SEXP matat, SEXP matFX, SEXP vecgX, SEXP ot, SEXP SDerror) {
    NumericMatrix matvt_n(matvt);
    arma::mat matrixVt(matvt_n.begin(), matvt_n.nrow(), matvt_n.ncol());

    NumericMatrix matF_n(matF);
    arma::mat matrixF(matF_n.begin(), matF_n.nrow(), matF_n.ncol(), false);

    NumericMatrix matw_n(matw);
    arma::rowvec rowvecW(matw_n.begin(), matw_n.ncol(), false);

    NumericMatrix yt_n(yt);
    arma::vec vecYt(yt_n.begin(), yt_n.nrow(), false);

    NumericMatrix vecg_n(vecg);
    arma::vec vecG(vecg_n.begin(), vecg_n.nrow(), false);

    unsigned int hor = as<unsigned int>(h);

    IntegerVector lagsModel_n(lagsModel);
    arma::uvec lags = as<arma::uvec>(lagsModel_n);

    char E = as<char>(Etype);
    char T = as<char>(Ttype);
    char S = as<char>(Stype);

    bool multi = as<bool>(multisteps);

    std::string CFtype = as<std::string>(CFt);

    double normalize = as<double>(normalizer);

    char fitterType = as<char>(fittertype);

    NumericMatrix matxt_n(matxt);
    arma::mat matrixXt(matxt_n.begin(), matxt_n.nrow(), matxt_n.ncol(), false);

    NumericMatrix matat_n(matat);
    arma::mat matrixAt(matat_n.begin(), matat_n.nrow(), matat_n.ncol());

    NumericMatrix matFX_n(matFX);
    arma::mat matrixFX(matFX_n.begin(), matFX_n.nrow(), matFX_n.ncol(), false);

    NumericMatrix vecgX_n(vecgX);
    arma::vec vecGX(vecgX_n.begin(), vecgX_n.nrow(), false);

    NumericVector ot_n(ot);
    arma::vec vecOt(ot_n.begin(), ot_n.size(), false);

    double errorSD = as<double>(SDerror);

    return wrap(optimizer(matrixVt, matrixF, rowvecW, vecYt, vecG,
                          hor, lags, E, T, S,
                          multi, CFtype, normalize, fitterType,
                          matrixXt, matrixAt, matrixFX, vecGX, vecOt, errorSD));
}

/* # Function is used in cases when the persistence vector needs to be estimated.
# If bounds are violated, it returns variance of yt. */
// [[Rcpp::export]]
RcppExport SEXP costfunc(SEXP matvt, SEXP matF, SEXP matw, SEXP yt, SEXP vecg,
                              SEXP h, SEXP lagsModel, SEXP Etype, SEXP Ttype, SEXP Stype,
                              SEXP multisteps, SEXP CFt, SEXP normalizer, SEXP fittertype,
                              SEXP matxt, SEXP matat, SEXP matFX, SEXP vecgX, SEXP ot,
                              SEXP bounds, SEXP SDerror) {
/* Function is needed to implement admissible constrains on smoothing parameters */
    NumericMatrix matvt_n(matvt);
    arma::mat matrixVt(matvt_n.begin(), matvt_n.nrow(), matvt_n.ncol());

    NumericMatrix matF_n(matF);
    arma::mat matrixF(matF_n.begin(), matF_n.nrow(), matF_n.ncol(), false);

    NumericMatrix matw_n(matw);
    arma::rowvec rowvecW(matw_n.begin(), matw_n.ncol(), false);

    NumericMatrix yt_n(yt);
    arma::vec vecYt(yt_n.begin(), yt_n.nrow(), false);

    NumericMatrix vecg_n(vecg);
    arma::vec vecG(vecg_n.begin(), vecg_n.nrow(), false);

    int hor = as<int>(h);

    IntegerVector lagsModel_n(lagsModel);
    arma::uvec lags = as<arma::uvec>(lagsModel_n);

    char E = as<char>(Etype);
    char T = as<char>(Ttype);
    char S = as<char>(Stype);

    bool multi = as<bool>(multisteps);

    std::string CFtype = as<std::string>(CFt);

    double normalize = as<double>(normalizer);

    char fitterType = as<char>(fittertype);

    NumericMatrix matxt_n(matxt);
    arma::mat matrixXt(matxt_n.begin(), matxt_n.nrow(), matxt_n.ncol(), false);

    NumericMatrix matat_n(matat);
    arma::mat matrixAt(matat_n.begin(), matat_n.nrow(), matat_n.ncol());

    NumericMatrix matFX_n(matFX);
    arma::mat matrixFX(matFX_n.begin(), matFX_n.nrow(), matFX_n.ncol(), false);

    NumericMatrix vecgX_n(vecgX);
    arma::vec vecGX(vecgX_n.begin(), vecgX_n.nrow(), false);

    NumericVector ot_n(ot);
    arma::vec vecOt(ot_n.begin(), ot_n.size(), false);

    char boundtype = as<char>(bounds);

    double errorSD = as<double>(SDerror);

    // Test the bounds for the ETS elements
    double boundsTestResult = boundsTester(boundtype, T, S, vecG, rowvecW, matrixF);
    if(boundsTestResult!=0){
        return wrap(boundsTestResult);
    }

    if(matrixAt(0,0)!=0){
        // Test the bounds for the explanatory part
        arma::rowvec rowvecWX(matFX_n.nrow(), arma::fill::ones);
        boundsTestResult = boundsTester('a', T, S, vecGX, rowvecWX, matrixFX);
    }
    if(boundsTestResult!=0){
        return wrap(boundsTestResult);
    }

    return wrap(optimizer(matrixVt, matrixF, rowvecW, vecYt, vecG,
                          hor, lags, E, T, S,
                          multi, CFtype, normalize, fitterType,
                          matrixXt, matrixAt, matrixFX, vecGX, vecOt, errorSD));
}