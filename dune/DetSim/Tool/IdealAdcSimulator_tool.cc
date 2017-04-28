// IdealAdcSimulator_tool.cc

#include "IdealAdcSimulator.h"

//**********************************************************************

IdealAdcSimulator::IdealAdcSimulator(double vsen, unsigned int nbit)
: m_vsen(vsen), m_vmax(0.0), m_adcmax(0) {
  if ( nbit == 0 ) return;
  unsigned int nbitmax = 8*sizeof(Count);
  if ( nbit > nbitmax ) return;
  unsigned int bit = 1;
  for ( unsigned int ibit=0; ibit<nbit; ++ibit ) {
    m_adcmax |= bit;
    bit = bit << 1;
  }
  m_vmax = m_vsen*m_adcmax;
}

//**********************************************************************

IdealAdcSimulator::IdealAdcSimulator(const fhicl::ParameterSet& ps)
: IdealAdcSimulator(ps.get<double>("Vsen"), ps.get<unsigned int>("Nbit")) { }

//**********************************************************************

AdcSimulator::Count
IdealAdcSimulator::count(double vin, Channel, Tick) const {
  if ( vin < 0.0 ) return 0;
  if ( m_vsen <= 0.0 ) return 0.0;
  if ( vin > m_vmax ) return m_adcmax;
  Count count = vin/m_vsen + 1;
  assert( count <- m_adcmax );
  if ( count > m_adcmax ) return m_adcmax;
  return count;
}

//**********************************************************************
