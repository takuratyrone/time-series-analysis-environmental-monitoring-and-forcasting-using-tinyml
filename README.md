# Time Series Analysis for Environmental Monitoring and Forcasting using Tinyml

## Abstract
TinyML is a technology that allows machine learning models to run on low-power devices
such as microcontrollers. The model is run locally on the device, eliminating the need to
run it on the cloud. This research aimed to investigate the use of TinyML in Environmental
Monitoring and forecasting and to produce a working prototype for this system. The
use case selected was temperature monitoring and forecasting to automate regulating
greenhouse environmental conditions.

The machine learning model was trained on the cloud, Google Colab, were there are
high computational resources, using Python’s TensorFlow library. The Convolutional
Neural Network (CNN), Multilayer Perceptron (MLP), and the Long-Short Term Memory
(LSTM) were used to train the model and were tested for prediction accuracy. Metrics of
accuracy used were the Root Mean Squared Error (RMSE) and the Mean Absolute Error
(MAE). The trained models were converted to their lightweight versions, TensorFlow
Lite, to be deployed on the target microcontroller for inference.

## Training model
The model was trained using the model_training.ipynb notebook. The data used to train the model is in training_data.csv

## Scripts
The scripts for collecting the training data and running the inference are found in the src/ directory
The script used to collect the training data is data_collection.ino
To run the inference: model_inference.ino
