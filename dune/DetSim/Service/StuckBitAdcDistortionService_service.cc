// StuckBitAdcDistortionService_service.cc

#include "dune/DetSim/Service/StuckBitAdcDistortionService.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Core/EngineCreator.h"
#include "artextensions/SeedService/SeedService.hh"
#include "CLHEP/Random/RandFlat.h"
#include "TFile.h"
#include "TString.h"
#include "TProfile.h"

using std::string;
using std::ostream;
using std::endl;

#undef UseSeedService

//**********************************************************************

StuckBitAdcDistortionService::
StuckBitAdcDistortionService(const fhicl::ParameterSet& pset, art::ActivityRegistry&)
: m_pran(nullptr) {
  // Fetch the random engine.
#ifdef UseSeedService
  int seed = seedSvc->getSeed("StuckBitAdcDistortionService");
#else
  int seed = 1009;
#endif
  art::EngineCreator ecr;
  m_pran = &ecr.createEngine(seed, "HepJamesRandom", "StuckBitsNoiseService");
  // Fetch parameters.
  fStuckBitsProbabilitiesFname = pset.get<string>("StuckBitsProbabilitiesFname");
  fStuckBitsOverflowProbHistoName = pset.get<string>("StuckBitsOverflowProbHistoName");
  fStuckBitsUnderflowProbHistoName = pset.get<string>("StuckBitsUnderflowProbHistoName");
  // Fetch the probabilities.
  mf::LogInfo("SimWireDUNE") << " using ADC stuck code probabilities from .root file " ;
  std::string fname;
  cet::search_path sp("FW_SEARCH_PATH");
  sp.find_file(fStuckBitsProbabilitiesFname, fname);
  std::unique_ptr<TFile> fin(new TFile(fname.c_str(), "READ"));
  if ( !fin->IsOpen() )
    throw art::Exception(art::errors::NotFound)
          << "Could not find the ADC stuck code probabilities file " << fname;
  TString iOverflowHistoName = Form( "%s", fStuckBitsOverflowProbHistoName.c_str());
  TProfile *overflowtemp = (TProfile*) fin->Get( iOverflowHistoName );
  if ( !overflowtemp )
    throw art::Exception(art::errors::NotFound)
          << "Could not find the ADC code overflow probabilities histogram "
          << fStuckBitsOverflowProbHistoName;
  if ( overflowtemp->GetNbinsX() != 64 )
    throw art::Exception(art::errors::InvalidNumber)
          << "Overflow ADC stuck code probability histograms must have 64 bins.";
  TString iUnderflowHistoName = Form( "%s", fStuckBitsUnderflowProbHistoName.c_str());
  TProfile *underflowtemp = (TProfile*) fin->Get(iUnderflowHistoName);
  if ( !underflowtemp )
    throw art::Exception( art::errors::NotFound )
          << "Could not find the ADC code underflow probabilities histogram "
          << fStuckBitsUnderflowProbHistoName;
  if ( underflowtemp->GetNbinsX() != 64 )
    throw art::Exception(art::errors::InvalidNumber)
          << "Underflow ADC stuck code probability histograms must have 64 bins.";
  for ( unsigned int cellnumber=0; cellnumber < 64; ++cellnumber ) {
    fOverflowProbs[cellnumber] = overflowtemp->GetBinContent(cellnumber+1);
    fUnderflowProbs[cellnumber] = underflowtemp->GetBinContent(cellnumber+1);
  }
  fin->Close();
}
  
//**********************************************************************

int StuckBitAdcDistortionService::modify(Channel, AdcCountVector& adcvec) const {
  CLHEP::RandFlat stuck_flat(*m_pran);
  for ( size_t itck = 0; itck<adcvec.size(); ++itck ) {
    double rnd = stuck_flat.fire(0,1);
    const unsigned int zeromask = 0xffc0;
    const unsigned int onemask = 0x003f;
    unsigned int sixlsbs = adcvec[itck] & onemask;
    int probability_index = (int)sixlsbs;
    if ( rnd < fUnderflowProbs[probability_index] ) {
      adcvec[itck] = adcvec[itck] | onemask; // 6 LSBs are stuck at 3F
      adcvec[itck] -= 64; // correct 1st MSB value by subtracting 64
    } else if ( rnd > fUnderflowProbs[probability_index] &&
              rnd < fUnderflowProbs[probability_index] + fOverflowProbs[probability_index] ) {
      adcvec[itck] = adcvec[itck] & zeromask; // 6 LSBs are stuck at 0
      adcvec[itck] += 64; // correct 1st MSB value by adding 64
    }
  }
  return 0;
}

//**********************************************************************

ostream& StuckBitAdcDistortionService::print(ostream& out, string prefix) const {
  out << prefix << "StuckBitAdcDistortionService:" << endl;
  out << prefix << "       StuckBitsProbabilitiesFname: " << fStuckBitsProbabilitiesFname << endl;
  out << prefix << "   fStuckBitsOverflowProbHistoName: " << fStuckBitsOverflowProbHistoName << endl;
  out << prefix << "  fStuckBitsUnderflowProbHistoName: " << fStuckBitsUnderflowProbHistoName << endl;
  return out;
}

//**********************************************************************

DEFINE_ART_SERVICE_INTERFACE_IMPL(StuckBitAdcDistortionService, AdcDistortionService)

//**********************************************************************
