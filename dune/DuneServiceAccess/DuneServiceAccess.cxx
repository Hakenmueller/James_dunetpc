// DuneServiceAccess.cxx

#include "DuneServiceAccess.h"

template<>
ChannelGroupService* ArtServicePointer<ChannelGroupService>() {
  return GenericArtServicePointer<ChannelGroupService>();
}

template<>
AdcSuppressService* ArtServicePointer<AdcSuppressService>() {
  return GenericArtServicePointer<AdcSuppressService>();
}

template<>
PedestalEvaluationService* ArtServicePointer<PedestalEvaluationService>() {
  return GenericArtServicePointer<PedestalEvaluationService>();
}

template<>
AdcNoiseRemovalService* ArtServicePointer<AdcNoiseRemovalService>() {
  return GenericArtServicePointer<AdcNoiseRemovalService>();
}

template<>
AdcChannelNoiseRemovalService* ArtServicePointer<AdcChannelNoiseRemovalService>() {
  return GenericArtServicePointer<AdcChannelNoiseRemovalService>();
}

template<>
AdcDeconvolutionService* ArtServicePointer<AdcDeconvolutionService>() {
  return GenericArtServicePointer<AdcDeconvolutionService>();
}

