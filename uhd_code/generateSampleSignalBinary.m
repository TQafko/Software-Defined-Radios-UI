% Generate binary files for signals 
% Tedi Qafko

clear all
close all
function [t,x]=signal(fc,fref,B,T,fsamp,name)
    % Use: [t,x]=signal(fc,fref,B,T,fsamp)
    % fc:    original center frequency (Hz)
    % fref:  frequency that the signal will be referenced to (Hz)
    % B:     bandwidth (Hz)
    % T:     pulsewidth (s)
    % fsamp: sample frequency (Hz)
    % name: name of file to output
    tsamp=1/fsamp;
    t=-T/2:tsamp:T/2;
    x=cos(2*pi*(fc-fref+B*t/(2*T)).*t) + 1i*sin(2*pi*(fc-fref+B*t/(2*T)).*t);
    
    % reference the time
    t = t + T/2;
    
    % save file
    save_name = sprintf('Signal_%s.bin', name);
    fp=fopen( fullfile( '.', 'data', save_name), 'wb' );
    fwrite(fp,reshape( round([real(x(:).');imag(x(:).')]*(2^15-1)), 1, [] ), 'int16' );
    fclose(fp);
    fprintf( 'File Name: %s\n', save_name );
    
end


