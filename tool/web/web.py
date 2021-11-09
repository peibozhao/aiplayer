#!/usr/bin/env python

import dash
from dash import html
from dash import dcc
import dash_bootstrap_components as dbc
from dash.dependencies import Input, Output
import requests
import base64
import cv2
import plotly.express as px
import numpy as np

import json


status_names = ['running', 'pause', 'stop']
mode_names = ['return', 'exercise', '11-4', 'hard-10-2']
host_port = 'http://localhost:9000'


app = dash.Dash(external_stylesheets=[dbc.themes.BOOTSTRAP])

app.layout = html.Div([
    dbc.Row([dbc.Col(html.H3('碧蓝航线设置', style={'textAlign': 'center'}), width=3)],
            justify='center', style={'margin': '10px'}),
    dbc.Row([dbc.Col(html.Div('状态'), align='center', width=1),
             dbc.Col(dbc.Button('查询', id='query_status'),
                     align='center', width=1),
             dbc.Col(dbc.DropdownMenu([dbc.DropdownMenuItem(i, id=i) for i in status_names],
                                      id='status_menu', size='lg'), align='center', width=1)],
            justify='center', style={'margin': '10px'}),
    dbc.Row([dbc.Col(html.Div('模式'), align='center', width=1),
             dbc.Col(dbc.Button('查询', id='query_mode'),
                     align='center', width=1),
             dbc.Col(dbc.DropdownMenu([dbc.DropdownMenuItem(i, id=i) for i in mode_names],
                                      id='mode_menu', size='lg'), align='center', width=1)],
            justify='center', style={'margin': '10px'}),
    dbc.Row([dbc.Col(html.Div('图像'), align='center', width=1),
             dbc.Col(dbc.Button('查询', id='query_image'), width=1)],
            justify='center'),
    dbc.Row([dbc.Col(dcc.Graph(id='image'))], justify='center'),
    html.Div(id='click_event')
])


@ app.callback(Output('status_menu', 'label'),
               [Input(i, 'n_clicks') for i in status_names] + [Input('query_status', 'n_clicks')])
def status(*args):
    ctx = dash.callback_context
    if not ctx.triggered:
        # Get status
        resp = requests.get(host_port + '/status')
        return str(resp.content, encoding='utf-8')

    button_id = ctx.triggered[0]['prop_id'].split('.')[0]
    if button_id == 'query_status':
        # Get status
        resp = requests.get(host_port + '/status')
        return str(resp.content, encoding='utf-8')
    else:
        requests.put(host_port + '/status', data=button_id)
    return button_id


@app.callback(Output('mode_menu', 'label'),
              [Input(i, 'n_clicks') for i in mode_names] + [Input('query_mode', 'n_clicks')])
def mode(*args):
    ctx = dash.callback_context
    if not ctx.triggered:
        # Get mode
        resp = requests.get(host_port + '/current_mode/name')
        return str(resp.content, encoding='utf-8')

    button_id = ctx.triggered[0]['prop_id'].split('.')[0]
    if button_id == 'query_mode':
        # Get mode
        resp = requests.get(host_port + '/current_mode/name')
        return str(resp.content, encoding='utf-8')
    else:
        requests.put(host_port + '/current_mode/name', data=button_id)
    return button_id


@app.callback(Output('image', 'figure'),
              Input('query_image', 'n_clicks'),
              prevent_initial_call=True)
def query_image(n_clicks):
    # Get image
    resp = requests.get(host_port + '/image')
    jpg = base64.b64decode(resp.content)
    img = cv2.imdecode(np.asarray(bytearray(jpg), dtype='uint8'), cv2.IMREAD_COLOR)
    return px.imshow(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))


@app.callback(
    Output('click_event', 'children'),
    Input('image', 'clickData'),
    prevent_initial_call=True,
)
def click(click_data):
    # Click image
    x = click_data['points'][0]['x']
    y = click_data['points'][0]['y']
    requests.put(host_port + '/operation', data=f'{x},{y}')
    return json.dumps(click_data)

app.run_server(host='0.0.0.0')
