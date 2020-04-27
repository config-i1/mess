// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <RcppArmadillo.h>
#include <Rcpp.h>

using namespace Rcpp;

// matrixPowerWrap
RcppExport SEXP matrixPowerWrap(SEXP matA, SEXP power);
RcppExport SEXP _mes_matrixPowerWrap(SEXP matASEXP, SEXP powerSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< SEXP >::type matA(matASEXP);
    Rcpp::traits::input_parameter< SEXP >::type power(powerSEXP);
    rcpp_result_gen = Rcpp::wrap(matrixPowerWrap(matA, power));
    return rcpp_result_gen;
END_RCPP
}
// adamFitterWrap
RcppExport SEXP adamFitterWrap(SEXP matVt, SEXP matWt, SEXP matF, SEXP vecG, SEXP lagsModelAll, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP componentsNumber, SEXP componentsNumberSeasonal, SEXP yInSample, SEXP ot, SEXP backcasting);
RcppExport SEXP _mes_adamFitterWrap(SEXP matVtSEXP, SEXP matWtSEXP, SEXP matFSEXP, SEXP vecGSEXP, SEXP lagsModelAllSEXP, SEXP EtypeSEXP, SEXP TtypeSEXP, SEXP StypeSEXP, SEXP componentsNumberSEXP, SEXP componentsNumberSeasonalSEXP, SEXP yInSampleSEXP, SEXP otSEXP, SEXP backcastingSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< SEXP >::type matVt(matVtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matWt(matWtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matF(matFSEXP);
    Rcpp::traits::input_parameter< SEXP >::type vecG(vecGSEXP);
    Rcpp::traits::input_parameter< SEXP >::type lagsModelAll(lagsModelAllSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Etype(EtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Ttype(TtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Stype(StypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumber(componentsNumberSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumberSeasonal(componentsNumberSeasonalSEXP);
    Rcpp::traits::input_parameter< SEXP >::type yInSample(yInSampleSEXP);
    Rcpp::traits::input_parameter< SEXP >::type ot(otSEXP);
    Rcpp::traits::input_parameter< SEXP >::type backcasting(backcastingSEXP);
    rcpp_result_gen = Rcpp::wrap(adamFitterWrap(matVt, matWt, matF, vecG, lagsModelAll, Etype, Ttype, Stype, componentsNumber, componentsNumberSeasonal, yInSample, ot, backcasting));
    return rcpp_result_gen;
END_RCPP
}
// adamForecasterWrap
RcppExport SEXP adamForecasterWrap(SEXP matVt, SEXP matWt, SEXP matF, SEXP lagsModelAll, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP componentsNumber, SEXP componentsNumberSeasonal, SEXP h);
RcppExport SEXP _mes_adamForecasterWrap(SEXP matVtSEXP, SEXP matWtSEXP, SEXP matFSEXP, SEXP lagsModelAllSEXP, SEXP EtypeSEXP, SEXP TtypeSEXP, SEXP StypeSEXP, SEXP componentsNumberSEXP, SEXP componentsNumberSeasonalSEXP, SEXP hSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< SEXP >::type matVt(matVtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matWt(matWtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matF(matFSEXP);
    Rcpp::traits::input_parameter< SEXP >::type lagsModelAll(lagsModelAllSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Etype(EtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Ttype(TtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Stype(StypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumber(componentsNumberSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumberSeasonal(componentsNumberSeasonalSEXP);
    Rcpp::traits::input_parameter< SEXP >::type h(hSEXP);
    rcpp_result_gen = Rcpp::wrap(adamForecasterWrap(matVt, matWt, matF, lagsModelAll, Etype, Ttype, Stype, componentsNumber, componentsNumberSeasonal, h));
    return rcpp_result_gen;
END_RCPP
}
// adamErrorerWrap
RcppExport SEXP adamErrorerWrap(SEXP matVt, SEXP matWt, SEXP matF, SEXP lagsModelAll, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP componentsNumber, SEXP componentsNumberSeasonal, SEXP h, SEXP yInSample, SEXP ot);
RcppExport SEXP _mes_adamErrorerWrap(SEXP matVtSEXP, SEXP matWtSEXP, SEXP matFSEXP, SEXP lagsModelAllSEXP, SEXP EtypeSEXP, SEXP TtypeSEXP, SEXP StypeSEXP, SEXP componentsNumberSEXP, SEXP componentsNumberSeasonalSEXP, SEXP hSEXP, SEXP yInSampleSEXP, SEXP otSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< SEXP >::type matVt(matVtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matWt(matWtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matF(matFSEXP);
    Rcpp::traits::input_parameter< SEXP >::type lagsModelAll(lagsModelAllSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Etype(EtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Ttype(TtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Stype(StypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumber(componentsNumberSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumberSeasonal(componentsNumberSeasonalSEXP);
    Rcpp::traits::input_parameter< SEXP >::type h(hSEXP);
    Rcpp::traits::input_parameter< SEXP >::type yInSample(yInSampleSEXP);
    Rcpp::traits::input_parameter< SEXP >::type ot(otSEXP);
    rcpp_result_gen = Rcpp::wrap(adamErrorerWrap(matVt, matWt, matF, lagsModelAll, Etype, Ttype, Stype, componentsNumber, componentsNumberSeasonal, h, yInSample, ot));
    return rcpp_result_gen;
END_RCPP
}
// adamSimulatorwrap
RcppExport SEXP adamSimulatorwrap(SEXP arrVt, SEXP matErrors, SEXP matOt, SEXP matF, SEXP matWt, SEXP matG, SEXP Etype, SEXP Ttype, SEXP Stype, SEXP lagsModelAll, SEXP componentsNumberSeasonal, SEXP componentsNumber);
RcppExport SEXP _mes_adamSimulatorwrap(SEXP arrVtSEXP, SEXP matErrorsSEXP, SEXP matOtSEXP, SEXP matFSEXP, SEXP matWtSEXP, SEXP matGSEXP, SEXP EtypeSEXP, SEXP TtypeSEXP, SEXP StypeSEXP, SEXP lagsModelAllSEXP, SEXP componentsNumberSeasonalSEXP, SEXP componentsNumberSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< SEXP >::type arrVt(arrVtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matErrors(matErrorsSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matOt(matOtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matF(matFSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matWt(matWtSEXP);
    Rcpp::traits::input_parameter< SEXP >::type matG(matGSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Etype(EtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Ttype(TtypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type Stype(StypeSEXP);
    Rcpp::traits::input_parameter< SEXP >::type lagsModelAll(lagsModelAllSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumberSeasonal(componentsNumberSeasonalSEXP);
    Rcpp::traits::input_parameter< SEXP >::type componentsNumber(componentsNumberSEXP);
    rcpp_result_gen = Rcpp::wrap(adamSimulatorwrap(arrVt, matErrors, matOt, matF, matWt, matG, Etype, Ttype, Stype, lagsModelAll, componentsNumberSeasonal, componentsNumber));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_mes_matrixPowerWrap", (DL_FUNC) &_mes_matrixPowerWrap, 2},
    {"_mes_adamFitterWrap", (DL_FUNC) &_mes_adamFitterWrap, 13},
    {"_mes_adamForecasterWrap", (DL_FUNC) &_mes_adamForecasterWrap, 10},
    {"_mes_adamErrorerWrap", (DL_FUNC) &_mes_adamErrorerWrap, 12},
    {"_mes_adamSimulatorwrap", (DL_FUNC) &_mes_adamSimulatorwrap, 12},
    {NULL, NULL, 0}
};

RcppExport void R_init_mes(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
