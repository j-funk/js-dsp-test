//
//  ViewController.m
//  native-fft-test
//
//  Created by Chris Cannam on 17/10/2015.
//  Copyright (c) 2015 Chris Cannam. All rights reserved.
//

#import "ViewController.h"

#include <vector>
#include <chrono>
#include <iostream>

#include "bqfft/FFT.h"

using namespace std;
using namespace breakfastquay;

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    vector<int> sizes { 512, 2048 };
    
    int iterations = 2000;
    int times = 100;
    
    for (auto size: sizes) {
        
        FFT fft(size);
        double total = 0.0;
        
        auto start = chrono::high_resolution_clock::now();
        
        for (int ti = 0; ti < times; ++ti) {
            
            total = 0.0;
            
            for (int i = 0; i < iterations; ++i) {
                
                vector<double> ri(size), ro(size/2+1), io(size/2+1);
                for (int j = 0; j < size; ++j) {
                    ri[j] = (j % 2) / 4.0;
                }
                
                fft.forward(ri.data(), ro.data(), io.data());
                
                for (int j = 0; j <= size/2; ++j) {
                    total += sqrt(ro[j] * ro[j] + io[j] * io[j]);
                }
                
                // synthesise the conjugate half
                for (int j = 1; j < size/2; ++j) {
                    total += sqrt(ro[j] * ro[j] + io[j] * io[j]);
                }
            }
        }
        
        auto end = chrono::high_resolution_clock::now();
        
        double ms = chrono::duration<double, milli>(end - start).count() / times;
        
        cerr << "for " << iterations << " * size " << size << ": total = "
        << total << ", time = " << ms
        << " ms (" << (iterations / (ms / 1000.0)) << " itr/sec)" << endl;
    }

    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
