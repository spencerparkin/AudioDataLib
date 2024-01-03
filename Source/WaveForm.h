#pragma once

#include "AudioData.h"

namespace AudioDataLib
{
	// TODO: Might be interesting to one day be able to convert to/from another form using FFT.
	class AUDIO_DATA_LIB_API WaveForm
	{
		friend class Index;

	public:
		WaveForm();
		virtual ~WaveForm();

		void ConvertFromAudioBuffer(const AudioData::Format& format, const uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel);
		void ConvertToAudioBuffer(const AudioData::Format& format, uint8_t* audioBuffer, uint64_t audioBufferSize, uint16_t channel) const;

		void Clear();

		// If an index exists, it can speed up evaluation.
		void GenerateIndex() const;

		double EvaluateAt(double timeSeconds) const;

		void SumTogether(const std::list<WaveForm*>& waveFormList);

		struct Sample
		{
			double timeSeconds;
			double amplitude;
			//uint32_t i;
		};

		struct SampleBounds
		{
			const Sample* minSample;
			const Sample* maxSample;
		};

		bool FindSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const;

		double AverageSampleRate() const;
		double GetStartTime() const;
		double GetEndTime() const;
		double GetTimespan() const;
		uint64_t GetNumSamples() const;

	protected:

		class Index
		{
		public:
			Index();
			virtual ~Index();

			void Build(const WaveForm& waveForm);
			void Clear();
			bool FindSampleBounds(double timeSeconds, SampleBounds& sampleBounds) const;

		private:
			class Node
			{
			public:
				Node();
				virtual ~Node();

				virtual bool FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) = 0;
			};

			class InternalNode : public Node
			{
			public:
				InternalNode(double partition);
				virtual ~InternalNode();

				virtual bool FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) override;

				Node* leftNode;
				Node* rightNode;
				double partition;
			};

			class LeafNode : public Node
			{
			public:
				LeafNode(Sample* sample);
				virtual ~LeafNode();

				virtual bool FindTightestSampleBounds(double timeSeconds, SampleBounds& sampleBounds) override;

				Sample* sample;
			};

			Node* GenerateNode(const std::vector<Sample*>& sampleArray);

			Node* rootNode;
		};

		mutable Index* index;

		// We assume the samples are all in order according to time.
		std::vector<Sample>* sampleArray;
	};
}