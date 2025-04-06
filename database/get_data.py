#!/usr/bin/env python3
import argparse
import boto3
from decimal import Decimal
from boto3.dynamodb.conditions import Attr

def main():
    parser = argparse.ArgumentParser(description="Retrieve data from a DynamoDB table with optional numeric filters.")
    parser.add_argument("--table", default="Exponents", help="Name of the DynamoDB table.")
    parser.add_argument("--constellationM", type=float, help="Filter by constellationM (numeric).")
    parser.add_argument("--nodesN", type=float, help="Filter by nodesN (numeric).")
    parser.add_argument("--SNR", type=float, help="Filter by signalNoiseRatio (numeric).")
    parser.add_argument("--transmissionRate", type=float, help="Filter by transmissionRate (numeric).")
    args = parser.parse_args()

    # Create a DynamoDB resource (adjust region_name if needed)
    dynamodb = boto3.resource('dynamodb', region_name='eu-north-1')
    table = dynamodb.Table(args.table)

    # Build a filter expression incrementally.
    # If the user provides a parameter, we filter on that attribute;
    # otherwise, we skip it (no filter).
    filter_expr = None

    if args.constellationM is not None:
        filter_expr = (Attr('constellationM').eq(Decimal(str(args.constellationM)))
                       if filter_expr is None 
                       else filter_expr & Attr('constellationM').eq(Decimal(str(args.constellationM))))

    if args.nodesN is not None:
        filter_expr = (Attr('nodesN').eq(Decimal(str(args.nodesN)))
                       if filter_expr is None
                       else filter_expr & Attr('nodesN').eq(Decimal(str(args.nodesN))))

    if args.SNR is not None:
        filter_expr = (Attr('signalNoiseRatio').eq(Decimal(str(args.SNR)))
                       if filter_expr is None
                       else filter_expr & Attr('signalNoiseRatio').eq(Decimal(str(args.SNR))))

    if args.transmissionRate is not None:
        filter_expr = (Attr('transmissionRate').eq(Decimal(str(args.transmissionRate)))
                       if filter_expr is None
                       else filter_expr & Attr('transmissionRate').eq(Decimal(str(args.transmissionRate))))

    # Prepare the scan parameters
    scan_kwargs = {}
    if filter_expr is not None:
        scan_kwargs['FilterExpression'] = filter_expr

    # Paginate through all results (DynamoDB scan can be paginated)
    items = []
    last_evaluated_key = None
    while True:
        if last_evaluated_key:
            scan_kwargs['ExclusiveStartKey'] = last_evaluated_key
        
        response = table.scan(**scan_kwargs)
        items.extend(response.get('Items', []))
        
        last_evaluated_key = response.get('LastEvaluatedKey')
        if not last_evaluated_key:
            break

    # Print results
    if not items:
        print("No matching items found.")
    else:
        print(f"Found {len(items)} matching item(s):")
        for idx, item in enumerate(items, start=1):
            errorExp = item.get('errorExponent')
            optRho   = item.get('optimalRho')
            print(f"Item #{idx}: errorExponent={errorExp}, optimalRho={optRho}")

if __name__ == "__main__":
    main()