
files = dir('*.wav');
freq_data  = {[0, 0];};
power_data = {[0, 0];};
power_fs   = {[0, 0];};
fs = 0;
count = 0;
for file = files'
    count = count + 1;
    figure()
    [y,fs] = audioread(file.name);
    [P1,f1] = periodogram(y,[],[],fs,'power');
    ydft = fft(y);
    ydft = ydft(1:length(y)/2+1);
    freq_data{count} = abs(real(ydft));
    power_data{count} = P1;
    power_fs{count} = f1;
    freq = 0:fs/length(y):fs/2;
    subplot(2,1,1)
    plot(f1,P1,'k')
    grid
    ylabel('P_1')
    title('Power Spectrum')

        % plot magnitude
        % subplot(211);
        % plot(freq,abs(ydft));
        % xlabel('Hz');
end



H = freq_data{1};
for data = 2:3
    H = intersect(H, freq_data{data});
end

%display intersection
H

for data = 1:count
    [pk1,lc1] = findpeaks(power_data{data}(:,1),'SortStr','descend','NPeaks',4);
    P1peakFreqs = power_fs{data}(lc1)
end

[Cxy,f] = mscohere(power_data{2}(:,1),power_data{3}(:,1),[],[],[],fs);
thresh = 0.95;
[pks,locs] = findpeaks(Cxy,'MinPeakHeight',thresh);
MatchingFreqs = f(locs)
figure
plot(f,Cxy)
ax = gca;
grid
xlabel('Frequency (Hz)')
title('Coherence Estimate')
ax.XTick = MatchingFreqs;
ax.YTick = thresh;
axis([0 350 0 1])




